#include "LevelEditor.h"

#include "../../../IO/BinarySerialization.h"
#include "../../../Rendering/Basic Rendering/ScreenClearer.h"
#include "../../../Rendering/Basic Rendering/RenderingState.h"
#include "../../../Rendering/GUI/GUI Elements/GUIPanel.h"

#include "../PageManager.h"
#include "GUIRoom.h"


namespace
{
    //The callback for when an item on the context menu is clicked.
    auto onContextMenu = [](ContextMenu::Options clicked, ContextMenu* menu)
    {
        switch (clicked)
        {
            //Call the corresponding UI event for each case.
            #define CASE(enumVal, callbackName, callbackArg) \
                case ContextMenu::Options::O_ ## enumVal: \
                    menu->Editor->OnButton_ ## callbackName(callbackArg); \
                    break;

            
            CASE(CREATE_ROOM, CreateRoom,)
            CASE(MOVE_ROOM, MoveRoom,)
            CASE(DELETE_ROOM, DeleteRoom,)
            CASE(PLACE_TEAM1, PlaceTeam1,)
            CASE(PLACE_TEAM2, PlaceTeam2,)
            CASE(SET_LIGHT_AMMO, SetSpawn, IT_AMMO_LIGHT)
            CASE(SET_HEAVY_AMMO, SetSpawn, IT_AMMO_HEAVY)
            CASE(SET_SPECIAL_AMMO, SetSpawn, IT_AMMO_SPECIAL)
            CASE(SET_LIGHT_WEAPON, SetSpawn, IT_WEAPON_LIGHT)
            CASE(SET_HEAVY_WEAPON, SetSpawn, IT_WEAPON_HEAVY)
            CASE(SET_SPECIAL_WEAPON, SetSpawn, IT_WEAPON_SPECIAL)
            CASE(SET_HEALTH, SetSpawn, IT_HEALTH)
            CASE(SET_NONE, SetSpawn, IT_NONE)
            CASE(SAVE, Save,)
            CASE(TEST, Test,)
            CASE(QUIT, Quit,)

            default:
                assert(false);
                break;


            #undef CASE
        }
    };
}

LevelEditor::LevelEditor(const std::string& levelFileName, PageManager* manager, std::string& err)
    : Page(manager), worldViewBounds(0.0f, 20.0f, 0.0f, 20.0f),
      windowSizeF(ToV2f(manager->GetWindowSize())),
      levelFile(LevelInfo::LevelFilesPath + levelFileName + ".lvl"),
      worldViewGrid(worldViewBounds, err),
      contextMenu(this, err, onContextMenu)
{
    //See if any fields failed their initialization.
    if (!Assert(err.empty(), "Error initializing editor fields", err))
    {
        return;
    }

    //Try to load in the level data.
    BinaryReader reader(true, levelFile);
    if (!Assert(reader.ErrorMessage.empty(), "Error reading in '" + levelFile + "'", reader.ErrorMessage))
    {
        return;
    }
    try
    {
        reader.ReadDataStructure(LevelData);
    }
    catch (int ex)
    {
        assert(ex == DataReader::EXCEPTION_FAILURE);
        Assert(false, "Error reading in data structure from level file", reader.ErrorMessage);
        return;
    }


    //Generate GUI stuff and initialize state.

    //TODO: Create "choose room to create" panel.
}
LevelEditor::~LevelEditor(void)
{
}


void LevelEditor::Update(Vector2i mousePos, float frameSeconds)
{
    bool leftMouse = sf::Mouse::isButtonPressed(sf::Mouse::Left),
         rightMouse = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    bool justPressedLeft = (leftMouse && !mouseLastFrame);


    //Calculate the world grid position of the mouse.
    Vector2f mousePosF = ToV2f(mousePos),
             windowSizeF = ToV2f(Manager->GetWindowSize());
    Vector2f posLerp(mousePosF.x / windowSizeF.x, mousePosF.y / windowSizeF.y);
    currentMouseWorldPos = Vector2f::Lerp(worldViewBounds.GetMinCorner(),
                                          worldViewBounds.GetMaxCorner(),
                                          posLerp);
    Vector2i gridPosI = currentMouseWorldPos.Floored();
    currentMouseGridPos = ToV2u(Vector2i(Mathf::Max(gridPosI.x, 0), Mathf::Max(gridPosI.y, 0)));
    
    //Get the room that the mouse is hovering over.
    unsigned int mousedRoom = LevelData.GetRoom(currentMouseGridPos);
    bool noMousedRoom = (mousedRoom == LevelData.Rooms.size());

    //Update the editor based on its current state.
    switch (currentState)
    {
        case ES_IDLE:
            if (leftMouse && !mouseLastFrame)
            {
                //If the player clicked on a room, pick it up and let him move it.
                if (!noMousedRoom)
                {
                    currentState = ES_PLACING_ROOM;
                    placingRoom_Room = LevelData.Rooms[mousedRoom];

                    LevelData.Rooms.erase(LevelData.Rooms.begin() + mousedRoom);
                    LevelData.RoomItems.erase(LevelData.RoomItems.begin() + mousedRoom);
                    LevelData.RoomOffsets.erase(LevelData.RoomOffsets.begin() + mousedRoom);
                }
            }
            else if (rightMouse)
            {
                currentState = ES_WAITING_ON_RIGHT_MOUSE;
                waitingOnRightMouse_StartPos = mousePos;
            }
            break;

        case ES_PLACING_ROOM:
            if (leftMouse && !mouseLastFrame)
            {
                //If this is a valid spot to stick the room, go ahead and place it down.
                if (LevelData.IsAreaFree(currentMouseGridPos,
                                         currentMouseGridPos+ placingRoom_Room.RoomGrid.GetDimensions(),
                                         true))
                {
                    LevelData.Rooms.push_back(placingRoom_Room);
                    LevelData.RoomItems.push_back(placingRoom_Item);
                    LevelData.RoomOffsets.push_back(currentMouseGridPos);

                    currentState = ES_IDLE;
                }
            }
            break;

        case ES_WAITING_ON_RIGHT_MOUSE:
            if (!rightMouse)
            {
                //If the player was placing a room, remove it.
                if (placingRoom_IsPlacing)
                {
                    placingRoom_IsPlacing = false;
                    currentState = ES_IDLE;
                }
                //Otherwise, see what he right-clicked on.
                else
                {
                    //The player right-clicked on a single spot, so bring up the context menu.
                    if (noMousedRoom)
                    {
                        contextMenu.SetUpForEmptySpace();
                    }
                    else
                    {
                        contextMenu.SetUpForRoom(mousedRoom);
                    }
                    //TODO: Position context menu (pick a corner of the menu to center on the mouse based on which corner of the screen the mouse is on) and set its GUIPanel as the current root element.
                    assert(false);

                    currentState = ES_CONTEXT_MENU;
                }
            }
            //Otherwise, if the mouse is still held down, see if it should count as "dragging" yet.
            else
            {
                const float minDragDist = 5.0f;
                if (mousePos.Distance(waitingOnRightMouse_StartPos) >= minDragDist)
                {
                    currentState = ES_PANNING;
                    panning_LastPos = waitingOnRightMouse_StartPos;
                }
            }
            break;

        case ES_PANNING: {
            //Calculate the change in world position and move the view bounds by that change.
            Vector2i deltaMPos = mousePos - panning_LastPos;
            Vector2f deltaWorldMPos((float)deltaMPos.x * worldViewBounds.GetXSize() /
                                        (float)Manager->GetWindowSize().x,
                                    (float)deltaMPos.y * worldViewBounds.GetYSize() /
                                        (float)Manager->GetWindowSize().y);
            worldViewBounds.Move(deltaWorldMPos);

            //Reset the mouse to the position it was originally at.
            sf::Mouse::setPosition(sf::Vector2i(panning_LastPos.x, panning_LastPos.y),
                                   *Manager->GetWindow());

            if (!rightMouse)
            {
                if (placingRoom_IsPlacing)
                {
                    currentState = ES_PLACING_ROOM;
                }
                else
                {
                    currentState = ES_IDLE;
                }
            }
        }   break;

        case ES_CONTEXT_MENU:
            //If the context menu was dismissed by the player (by clicking outside it), go back to idling.
            if (contextMenu.GetIsExtended())
            {
                currentState = ES_IDLE;
            }
            break;

        //TODO: Implement the "ES_CREATING_ROOM" case.
        default:
            assert(false);
            break;
    }

    mouseLastFrame = leftMouse;

    Page::Update(mousePos, frameSeconds);
}
void LevelEditor::Render(float frameSeconds)
{
    //Set up rendering state.

    ScreenClearer(true, true, false, Vector4f(0.02f, 0.02f, 0.15f, 0.0f)).ClearScreen();
    RenderingState(RenderingState::C_NONE).EnableState();

    Vector2f windowSize = ToV2f(Manager->GetWindowSize());

    Camera guiCam;
    guiCam.SetPosition(Vector3f());
    guiCam.SetRotation(Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, -1.0f, 0.0f));
    guiCam.MinOrthoBounds = Vector3f(0.0f, 0.0f, -10.0f);
    guiCam.MaxOrthoBounds = Vector3f(windowSize, 10.0f);
    guiCam.PerspectiveInfo.Width = windowSize.x;
    guiCam.PerspectiveInfo.Height = windowSize.y;

    Matrix4f viewM, projM;
    guiCam.GetViewTransform(viewM);
    guiCam.GetOrthoProjection(projM);
    RenderInfo info(Manager->GetTotalElapsedSeconds(), &guiCam, &viewM, &projM);


    //Render the grid.
    worldViewGrid.SetBounds(Box2D(0.0f, windowSize.x, -windowSize.y, 0.0f));
    worldViewGrid.Render(frameSeconds, info);

    //Render the rooms.
    for (unsigned int i = 0; i < LevelData.Rooms.size(); ++i)
    {
        RenderRoom(LevelData.Rooms[i], LevelData.RoomItems[i], LevelData.RoomOffsets[i],
                   false, frameSeconds, info);
    }

    //Render whatever GUI is on top.
    if (GUIManager.RootElement.get() != 0)
    {
        GUIManager.Render(frameSeconds, info);
    }
}


void LevelEditor::OnWindowResized(void)
{
    windowSizeF = ToV2f(Manager->GetWindowSize());

    //TODO: Re-position the rest of the GUI elements.
}
void LevelEditor::OnCloseWindow(void)
{
    //TODO: Ask if the player is sure he wants to quit. Reuse assets from other menus.
    Manager->EndWorld();
}

void LevelEditor::OnOtherWindowEvent(sf::Event& windowEvent)
{
    //If the player used the mouse wheel, zoom in/out.
    if (windowEvent.type == sf::Event::MouseWheelMoved)
    {
        const float zoomChange = 1.1f;
        const float minDiagonal = 1.0f;
        if (windowEvent.mouseWheel.delta < 0 || worldViewBounds.GetDimensions().Length() > minDiagonal)
        {
            float scaleAmnt = powf(zoomChange, (float)windowEvent.mouseWheel.delta);
            worldViewBounds.Scale(Vector2f(scaleAmnt, scaleAmnt));
        }
    }
}


void LevelEditor::RenderRoom(const RoomInfo& room, ItemTypes spawnItem, Vector2u roomOffset,
                             bool isPlacing, float frameSeconds, const RenderInfo& info) const
{
    GUIRoom guiRoom(worldViewGrid, *this, isPlacing, &room, spawnItem);

    Vector2f minRoom_Screen = ToV2f(roomOffset);
    guiRoom.SetBounds(Box2D(minRoom_Screen.x, minRoom_Screen.y, Vector2f(1.0f, 1.0f)));

    guiRoom.Render(frameSeconds, info);
}


//UI callbacks.

void LevelEditor::OnButton_CreateRoom(void)
{
    //TODO: Set up and position the "create room" panel.
}
void LevelEditor::OnButton_MoveRoom(void)
{
    unsigned int room = LevelData.GetRoom(currentMouseGridPos);
    assert(room != LevelData.Rooms.size());

    placingRoom_Room = LevelData.Rooms[room];
    placingRoom_Item = LevelData.RoomItems[room];
    placingRoom_IsPlacing = true;

    LevelData.Rooms.erase(LevelData.Rooms.begin() + room);
    LevelData.RoomItems.erase(LevelData.RoomItems.begin() + room);
    LevelData.RoomOffsets.erase(LevelData.RoomOffsets.begin() + room);
}
void LevelEditor::OnButton_DeleteRoom(void)
{
    unsigned int room = LevelData.GetRoom(currentMouseGridPos);
    assert(room != LevelData.Rooms.size());

    LevelData.Rooms.erase(LevelData.Rooms.begin() + room);
    LevelData.RoomItems.erase(LevelData.RoomItems.begin() + room);
    LevelData.RoomOffsets.erase(LevelData.RoomOffsets.begin() + room);
}
void LevelEditor::OnButton_PlaceTeam1(void)
{
    LevelData.TeamOneCenter = currentMouseWorldPos;
}
void LevelEditor::OnButton_PlaceTeam2(void)
{
    LevelData.TeamTwoCenter = currentMouseWorldPos;
}
void LevelEditor::OnButton_SetSpawn(ItemTypes type)
{
    unsigned int room = LevelData.GetRoom(currentMouseGridPos);
    assert(room != LevelData.Rooms.size());
    assert(LevelData.Rooms[room].GetRoomHasSpawns());

    LevelData.RoomItems[room] = type;
}
void LevelEditor::OnButton_Save(void)
{
    BinaryWriter writer(true, sizeof(LevelInfo));

    writer.WriteDataStructure(LevelData, "Data");

    std::string err = writer.SaveData(levelFile);
    if (!Assert(err.empty(), "Error saving level '" + levelFile + "'", err))
    {
        return;
    }
}
void LevelEditor::OnButton_Test(void)
{
    //TODO: Create a "TestGame" page.
    assert(false);
}
void LevelEditor::OnButton_Quit(void)
{
    //TODO: Show "are you sure you want to quit?" panel.
    assert(false);
}

Vector2f LevelEditor::WorldPosToScreen(Vector2f worldPos) const
{
    Vector2f screenSizeF = ToV2f(Manager->GetWindowSize());

    Vector2f worldViewMin = worldViewBounds.GetMinCorner(),
             worldViewSize = worldViewBounds.GetDimensions();
    Vector2f posLerp((worldPos.x - worldViewMin.x) / worldViewSize.x,
                     (worldPos.y - worldViewMin.y) / worldViewSize.y);

    return Vector2f::Lerp(Vector2f(0.0f, -screenSizeF.x),
                          Vector2f(screenSizeF.x, 0.0f),
                          posLerp);
}
Vector2f LevelEditor::WorldSizeToScreen(Vector2f worldSize) const
{
    Vector2f worldViewSize = worldViewBounds.GetDimensions(),
             worldLerp(worldSize.x / worldViewSize.x,
                       worldSize.y / worldViewSize.y);

    return worldLerp.ComponentProduct(ToV2f(Manager->GetWindowSize()));
}
Vector2f LevelEditor::ScreenPosToWorld(Vector2f screenPos) const
{
    Vector2f windowSizeF = ToV2f(Manager->GetWindowSize()),
             screenSizeMin(0.0f, -windowSizeF.y);

    Vector2f posLerp((screenPos.x - screenSizeMin.x) / windowSizeF.x,
                     (screenPos.y - screenSizeMin.y) / windowSizeF.y);
    assert(posLerp.y >= 0.0f);

    return Vector2f::Lerp(worldViewBounds.GetMinCorner(),
                          worldViewBounds.GetMaxCorner(),
                          posLerp);
}
Vector2f LevelEditor::ScreenSizeToWorld(Vector2f screenSize) const
{
    Vector2f windowSizeF = ToV2f(Manager->GetWindowSize());

    Vector2f sizeLerp(screenSize.x / windowSizeF.x,
                      screenSize.y / windowSizeF.y);
    return sizeLerp.ComponentProduct(worldViewBounds.GetDimensions());
}
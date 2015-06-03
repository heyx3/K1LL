#include "LevelEditor.h"

#include "../../../IO/BinarySerialization.h"
#include "../../../Rendering/Basic Rendering/ScreenClearer.h"
#include "../../../Rendering/Basic Rendering/RenderingState.h"
#include "../../../Rendering/GUI/GUI Elements/GUIPanel.h"

#include "../PageManager.h"
#include "../ChooseLevelEditor.h"
#include "GUIRoom.h"

#include "../../../DebugAssist.h"


namespace
{
    //Depth constants.
    const float DEPTH_Grid = -0.5f,
                DEPTH_NormalRoom = -0.49f,
                DEPTH_SelectedRoom = -0.3f,
                DEPTH_PathingOverlay = -0.04f,
                DEPTH_ContextMenu = 0.5f;

    //Color constants.
    const Vector4f COLOR_NormalRoom(1.0f, 1.0f, 1.0f, 1.0f),
                   COLOR_MousedOverRoom(2.0f, 2.0f, 2.0f, 1.0f),
                   COLOR_InvalidRoom(4.0f, 2.0f, 2.0f, 1.0f),
                   COLOR_Grid(0.7f, 0.7f, 0.7f, 1.0f);



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
    : Page(manager),
      worldViewBounds(-0.5f, manager->GetWindowSize().x * 0.05f, -0.5f, manager->GetWindowSize().y * 0.05f),
      windowSizeF(ToV2f(manager->GetWindowSize())),
      levelFile(LevelInfo::LevelFilesPath + levelFileName + ".lvl"),
      worldViewGrid(worldViewBounds, err),
      contextMenu(this, err, onContextMenu),
      newRoomSelection(worldViewGrid, *this, err),
      levelPathing(*this),
      placingRoom_IsPlacing(false),
      placingRoom_Room(Array2D<BlockTypes>(1, 1, BT_NONE), Vector2u(), IT_NONE, 0.0f)
{
    //See if any fields failed their initialization.
    if (!Assert(err.empty(), "Error initializing editor page's fields", err))
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

    //TODO: Call "OnRoomAdded()" on GUILevelPathing instance. Set its depth to a constant value.
    levelPathing.OnRoomsChanged();
    levelPathing.Depth = DEPTH_PathingOverlay;


    //Set up GUI stuff.
    contextMenu.Depth = DEPTH_ContextMenu;
    worldViewGrid.SetColor(COLOR_Grid);
}
LevelEditor::~LevelEditor(void)
{
}


void LevelEditor::Update(Vector2i mousePos, float frameSeconds)
{
    if (!GetIsWindowInFocus())
    {
        return;
    }


    bool leftMouse = sf::Mouse::isButtonPressed(sf::Mouse::Left),
         rightMouse = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    bool justPressedLeft = (leftMouse && !mouseLastFrame);


    //Calculate the world grid position of the mouse.
    Vector2f mousePosF = ToV2f(mousePos),
             windowSizeF = ToV2f(Manager->GetWindowSize());
    Vector2f mousePosLerp(mousePosF.x / windowSizeF.x, -mousePosF.y / windowSizeF.y);
    currentMouseWorldPos = Vector2f::Lerp(worldViewBounds.GetMinCorner(),
                                          worldViewBounds.GetMaxCorner(),
                                          mousePosLerp);
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
                    placingRoom_IsPlacing = true;
                    placingRoom_Room = LevelData.Rooms[mousedRoom];

                    LevelData.Rooms.erase(LevelData.Rooms.begin() + mousedRoom);
                }
            }
            else if (rightMouse)
            {
                currentState = ES_WAITING_ON_RIGHT_MOUSE;
                waitingOnRightMouse_StartPos = mousePos;
            }
            break;

        case ES_PLACING_ROOM:
            placingRoom_Room.MinCornerPos = currentMouseGridPos;

            if (leftMouse && !mouseLastFrame)
            {
                //If this is a valid spot to stick the room, go ahead and place it down.
                Vector2u roomEnd = currentMouseGridPos +
                                   placingRoom_Room.Walls.GetDimensions() -
                                   Vector2u(1, 1);
                if (LevelData.IsAreaFree(currentMouseGridPos, roomEnd, true))
                {
                    LevelData.Rooms.push_back(placingRoom_Room);
                    levelPathing.OnRoomsChanged();

                    placingRoom_IsPlacing = false;
                    currentState = ES_IDLE;
                }
            }
            else if (rightMouse)
            {
                currentState = ES_WAITING_ON_RIGHT_MOUSE;
                waitingOnRightMouse_StartPos = mousePos;
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

                    levelPathing.OnRoomsChanged();
                }
                //Otherwise, see what he right-clicked on.
                else
                {
                    //The player right-clicked on a single spot, so bring up the context menu.
                    if (noMousedRoom)
                    {
                        contextMenu.SetUpForEmptySpace();
                        contextMenu_SelectedRoom = LevelData.Rooms.size();
                    }
                    else
                    {
                        contextMenu.SetUpForRoom(mousedRoom);
                        contextMenu_SelectedRoom = mousedRoom;
                    }

                    //Activate the context menu.
                    contextMenu.SetIsExtended(true);
                    Vector2f dims = contextMenu.GetBounds().GetDimensions();
                    GUIManager.SetRoot(&contextMenu);
                    
                    //Position the context menu.
                    //TODO: Scale if it's too big for the screen.
                    contextMenu.SetBounds(Box2D(0.0f, dims.x, -dims.y + 0.0f, 0.0f));

                    currentState = ES_CONTEXT_MENU;
                }
            }
            //Otherwise, if the mouse is still held down, see if it should count as "dragging" yet.
            else
            {
                const float minDragDist = 10.0f;
                if (mousePos.Distance(waitingOnRightMouse_StartPos) >= minDragDist)
                {
                    currentState = ES_PANNING;
                    Manager->GetWindow()->setMouseCursorVisible(false);
                    panning_LastPos = waitingOnRightMouse_StartPos;
                }
            }
            break;

        case ES_PANNING: {
            //Calculate the change in world position and move the view bounds by that change.
            Vector2i deltaMPos = mousePos - panning_LastPos;
            Vector2f deltaWorldMPos(-(float)deltaMPos.x * worldViewBounds.GetXSize() /
                                        (float)Manager->GetWindowSize().x,
                                    (float)deltaMPos.y * worldViewBounds.GetYSize() /
                                        (float)Manager->GetWindowSize().y);
            worldViewBounds.Move(deltaWorldMPos);

            //Reset the mouse to the position it was originally at.
            sf::Mouse::setPosition(sf::Vector2i(panning_LastPos.x,
                                                panning_LastPos.y + Manager->GetWindowSize().y),
                                   *Manager->GetWindow());

            if (!rightMouse)
            {
                Manager->GetWindow()->setMouseCursorVisible(true);

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
            if (!contextMenu.GetIsExtended())
            {
                currentState = ES_IDLE;
                GUIManager.SetRoot(0);
            }
            //TODO: Track right mouse previous frame, see if just right-clicked, and if so, go to ES_WAITING_ON_RIGHT_MOUSE.
            break;

        case ES_CREATING_ROOM:
            //If the player right-clicks, go back to idling.
            if (rightMouse)
            {
                currentState = ES_IDLE;
                GUIManager.SetRoot(0);
            }
            break;

        default:
            assert(false);
            break;
    }

    levelPathing.Update(frameSeconds, mousePosF);

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
    guiCam.MinOrthoBounds = Vector3f(0.0f, 0.0f, -1.0f);
    guiCam.MaxOrthoBounds = Vector3f(windowSize, 10.0f);
    guiCam.PerspectiveInfo.Width = windowSize.x;
    guiCam.PerspectiveInfo.Height = windowSize.y;

    Matrix4f viewM, projM;
    guiCam.GetViewTransform(viewM);
    guiCam.GetOrthoProjection(projM);
    RenderInfo info(Manager->GetTotalElapsedSeconds(), &guiCam, &viewM, &projM);


    //Render the grid.
    worldViewGrid.Depth = DEPTH_Grid;
    worldViewGrid.SetBounds(Box2D(0.0f, windowSize.x, -windowSize.y, 0.0f));
    worldViewGrid.Render(frameSeconds, info);

    //Render any rooms inside the view bounds.
    unsigned int mousedRoom = (placingRoom_IsPlacing ? 
                                   LevelData.Rooms.size() :
                                   LevelData.GetRoom(currentMouseGridPos));
    LevelInfo::UIntBox viewBnds;
    Vector2i tempMax = Vector2i(999999999, 999999999);
    viewBnds.Min = ToV2u(ToV2i(worldViewBounds.GetMinCorner()).Clamp(Vector2i(0, 0), tempMax));
    viewBnds.Max = ToV2u((ToV2i(worldViewBounds.GetMaxCorner()) + Vector2i(1, 1)).Clamp(Vector2i(0, 0),
                                                                                        tempMax));
    for (unsigned int i = 0; i < LevelData.Rooms.size(); ++i)
    {
        LevelInfo::UIntBox roomBnds = LevelData.GetBounds(i);
        if (LevelInfo::BoxesTouching(roomBnds.Min, roomBnds.Max, viewBnds.Min, viewBnds.Max))
        {
            bool isSelected = (mousedRoom == i);
            Vector4f col = isSelected ? COLOR_MousedOverRoom : COLOR_NormalRoom;
            float depth = isSelected ? DEPTH_SelectedRoom : DEPTH_NormalRoom;
            RenderRoom(LevelData.Rooms[i], isSelected, col, depth, frameSeconds, info);
        }
    }


    //State-specific rendering.
    switch (currentState)
    {
        case ES_IDLE:
        case ES_CREATING_ROOM: //Handled by the GUIManager.
        case ES_CONTEXT_MENU: //Handled by the GUIManager.
            break;

        //Several different states may be entered while the player is placing rooms.
        case ES_PLACING_ROOM:
        case ES_WAITING_ON_RIGHT_MOUSE:
        case ES_PANNING:
            if (placingRoom_IsPlacing)
            {
                Vector2u roomEnd = currentMouseGridPos +
                                   placingRoom_Room.Walls.GetDimensions() -
                                   Vector2u(1, 1);
                Vector4f blendCol = LevelData.IsAreaFree(currentMouseGridPos, roomEnd, true) ?
                                        COLOR_MousedOverRoom :
                                        COLOR_InvalidRoom;

                RenderRoom(placingRoom_Room, true, blendCol, DEPTH_NormalRoom, frameSeconds, info);
            }
            break;

        default:
            assert(false);
            break;
    }


    //Render whatever GUI is on top.
    levelPathing.Render(frameSeconds, info);
    if (GUIManager.GetRoot() != 0)
    {
        GUIManager.Render(frameSeconds, info);
    }
}


void LevelEditor::OnWindowResized(void)
{
    windowSizeF = ToV2f(Manager->GetWindowSize());

    float worldBoundsAR = worldViewBounds.GetXSize() / worldViewBounds.GetYSize(),
          windowAR = (float)Manager->GetWindowSize().x / (float)Manager->GetWindowSize().y;
    worldViewBounds.Scale(Vector2f(windowAR / worldBoundsAR, 1.0f));

    //Dismiss the context menu if it exists.
    if (currentState == ES_CONTEXT_MENU)
    {
        currentState = ES_IDLE;
        contextMenu.SetIsExtended(false);
        GUIManager.SetRoot(0);
    }

    newRoomSelection.OnWindowResized();
}
void LevelEditor::OnCloseWindow(void)
{
    Manager->EndWorld();
}

void LevelEditor::OnOtherWindowEvent(sf::Event& windowEvent)
{
    //If the player used the mouse wheel, zoom in/out.
    if (windowEvent.type == sf::Event::MouseWheelMoved)
    {
        const float zoomChange = 0.9f;
        const float minDiagonal = 1.0f;
        if (windowEvent.mouseWheel.delta < 0 || worldViewBounds.GetDimensions().Length() > minDiagonal)
        {
            float scaleAmnt = powf(zoomChange, (float)windowEvent.mouseWheel.delta);
            worldViewBounds.Scale(Vector2f(scaleAmnt, scaleAmnt));
        }
    }
}


void LevelEditor::RenderRoom(const LevelInfo::RoomData& room, bool isPlacing,
                             Vector4f blendCol, float depth,
                             float frameSeconds, const RenderInfo& info)
{
    GUIRoom guiRoom(worldViewGrid, *this, isPlacing, &room);

    guiRoom.Depth = depth;

    Vector2f minRoom_Screen = WorldPosToScreen(ToV2f(room.MinCornerPos)),
             sizeRoom_Screen = WorldSizeToScreen(ToV2f(room.Walls.GetDimensions()));
    guiRoom.SetBounds(Box2D(minRoom_Screen.x, minRoom_Screen.y - sizeRoom_Screen.y,
                            sizeRoom_Screen));

    guiRoom.SetColor(blendCol);
    guiRoom.Render(frameSeconds, info);
}


//UI callbacks.

void LevelEditor::OnRoomSelection(unsigned int index)
{
    GUIManager.SetRoot(0);

    placingRoom_Room = newRoomSelection.GetRooms()[index];
    placingRoom_IsPlacing = true;
    currentState = ES_PLACING_ROOM;
}

void LevelEditor::OnButton_CreateRoom(void)
{
    currentState = ES_CREATING_ROOM;
    GUIManager.SetRoot(&newRoomSelection);
}
void LevelEditor::OnButton_MoveRoom(void)
{
    currentState = ES_PLACING_ROOM;
    placingRoom_IsPlacing = true;
    placingRoom_Room = LevelData.Rooms[contextMenu_SelectedRoom];
    LevelData.Rooms.erase(LevelData.Rooms.begin() + contextMenu_SelectedRoom);
}
void LevelEditor::OnButton_DeleteRoom(void)
{
    LevelData.Rooms.erase(LevelData.Rooms.begin() + contextMenu_SelectedRoom);
    
    if (LevelData.Team1Base >= LevelData.Rooms.size())
    {
        LevelData.Team1Base = LevelData.Rooms.size() - 1;
    }
    if (LevelData.Team2Base >= LevelData.Rooms.size())
    {
        LevelData.Team2Base = LevelData.Rooms.size() - 1;
    }

    levelPathing.OnRoomsChanged();
}
void LevelEditor::OnButton_PlaceTeam1(void)
{
    LevelData.Team1Base = contextMenu_SelectedRoom;
}
void LevelEditor::OnButton_PlaceTeam2(void)
{
    LevelData.Team2Base = contextMenu_SelectedRoom;
}
void LevelEditor::OnButton_SetSpawn(ItemTypes type)
{
    assert(contextMenu_SelectedRoom < LevelData.Rooms.size());

    LevelData.Rooms[contextMenu_SelectedRoom].SpawnedItem = type;
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
    std::string err;
    Page::Ptr nextPage(new ChooseLevelEditor(Manager, err));
    if (!Assert(err.empty(), "Error setting up 'ChooseLevelEditor' page", err))
    {
        return;
    }

    Manager->UpdateCurrentPage(nextPage);
}


Vector2f LevelEditor::WorldPosToScreen(Vector2f worldPos) const
{
    Vector2f screenSizeF = ToV2f(Manager->GetWindowSize());

    Vector2f worldViewMin = worldViewBounds.GetMinCorner(),
             worldViewSize = worldViewBounds.GetDimensions();
    Vector2f posLerp((worldPos.x - worldViewMin.x) / worldViewSize.x,
                     (worldPos.y - worldViewMin.y) / worldViewSize.y);

    return Vector2f::Lerp(Vector2f(0.0f, 0.0f),
                          Vector2f(screenSizeF.x, -screenSizeF.y),
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
             screenSizeMin(0.0f, 0.0f);

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
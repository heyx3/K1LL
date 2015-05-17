#include "LevelEditor.h"

#include "../../../IO/BinarySerialization.h"
#include "../../../Rendering/GUI/GUI Elements/GUIPanel.h"

#include "../PageManager.h"
#include "GUIEditorGrid.h"


LevelEditor::LevelEditor(const std::string& levelFileName, PageManager* manager, std::string& err)
    : Page(manager), worldViewBounds(0.0f, 20.0f, 0.0f, 20.0f),
      noiseTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PS_8U, false)
{
    //Try to load in the level data.
    std::string fullPath = LevelInfo::LevelFilesPath + levelFileName + ".lvl";
    BinaryReader reader(true, fullPath);
    if (!Assert(reader.ErrorMessage.empty(), "Error reading in '" + fullPath + "'", reader.ErrorMessage))
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


    //Load noise texture.
    noiseTex.Create();
    if (!Assert(noiseTex.SetDataFromFile("Content/Menu/GridNoise.png", err),
                "Error loading 'Content/Menu/GridNoise.png", err))
    {
        return;
    }


    //Generate GUI stuff and initialize state.

    worldViewGrid = GUIElementPtr(new GUIEditorGrid(worldViewBounds, &noiseTex, err));
    if (!Assert(err.empty(), "Error setting up world view bounds", err))
    {
        return;
    }

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
    contextMenu_Menu = GUIElementPtr(new ContextMenu(this, err, onContextMenu));
    if (!Assert(err.empty(), "Error setting up context menu", err))
    {
        return;
    }

    GUIPanel* contextMenuPanel = new GUIPanel();
    contextMenuPanel->AddElement(worldViewGrid);
    contextMenuPanel->AddElement(contextMenu_Menu);
    contextMenu_MenuPanel = GUIElementPtr(contextMenuPanel);

    //TODO: Create "choose room to create" panel and some kind of Room GUIElement.
}
LevelEditor::~LevelEditor(void)
{
    //TODO: Implement any necessary cleanup.
}


void LevelEditor::Update(Vector2i mousePos, float frameSeconds)
{
    //TODO: Only count a click for menus, etc. if it hasn't been pressed last frame. Maybe update the GUI manager yourself instead of calling Page::Update().
    bool leftMouse = sf::Mouse::isButtonPressed(sf::Mouse::Left),
         rightMouse = sf::Mouse::isButtonPressed(sf::Mouse::Right);

    //Calculate the world grid position of the mouse.
    Vector2f mousePosF = ToV2f(mousePos),
             windowSizeF = ToV2f(Manager->GetWindowSize());
    Vector2f posLerp(mousePosF.x / windowSizeF.x, mousePosF.y / windowSizeF.y);
    Vector2i gridPosI = Vector2f::Lerp(worldViewBounds.GetMinCorner(), worldViewBounds.GetMaxCorner(),
                                       posLerp).Floored();
    Vector2u gridPos = ToV2u(Vector2i(Mathf::Max(gridPosI.x, 0), Mathf::Max(gridPosI.y, 0)));
    
    //Get the room that the mouse is hovering over.
    unsigned int mousedRoom = LevelData.GetRoom(gridPos);
    bool noMousedRoom = (mousedRoom == LevelData.Rooms.size());

    //Update the editor based on its current state.
    switch (currentState)
    {
        case ES_IDLE:
            if (leftMouse)
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
            if (leftMouse)
            {
                //If this is a valid spot to stick the room, go ahead and place it down.
                if (LevelData.IsAreaFree(gridPos, gridPos + placingRoom_Room.RoomGrid.GetDimensions(),
                                         true))
                {
                    LevelData.Rooms.push_back(placingRoom_Room);
                    LevelData.RoomItems.push_back(IT_NONE);
                    LevelData.RoomOffsets.push_back(gridPos);

                    currentState = ES_IDLE;
                }
            }
            break;

        case ES_WAITING_ON_RIGHT_MOUSE:
            if (!rightMouse)
            {
                //The player right-clicked on a single spot, so bring up the context menu.
                if (noMousedRoom)
                {
                    GetContextMenu().SetUpForEmptySpace();
                }
                else
                {
                    GetContextMenu().SetUpForRoom(mousedRoom);
                }
                //TODO: Position context menu (pick a corner of the menu to center on the mouse based on which corner of the screen the mouse is on) and set its GUIPanel as the current root element.
                assert(false);

                currentState = ES_CONTEXT_MENU;
            }
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
                currentState = ES_IDLE;
            }
        } break;

        case ES_CONTEXT_MENU:
            //If the context menu was dismissed by the player (by clicking outside it), go back to idling.
            if (!GetContextMenu().GetIsExtended())
            {
                currentState = ES_IDLE;
            }
            break;

        //TODO: Implement the "ES_CREATING_ROOM" case.
        default:
            assert(false);
            break;
    }

    Page::Update(mousePos, frameSeconds);
}
void LevelEditor::Render(float frameSeconds)
{
    Page::Render(frameSeconds);
}


void LevelEditor::OnWindowResized(void)
{
    Vector2f windowSize = ToV2f(Manager->GetWindowSize());
    worldViewGrid->SetBounds(Box2D(0.0f, windowSize.x, -windowSize.y, 0.0f));

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
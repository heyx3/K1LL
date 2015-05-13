#include "LevelEditor.h"

#include "../PageManager.h"
#include "../../../IO/BinarySerialization.h"


LevelEditor::LevelEditor(const std::string& levelFileName, PageManager* manager, std::string& err)
    : Page(manager), worldViewBounds(0.0f, 20.0f, 0.0f, 20.0f)
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


    //Generate GUI stuff and initialize state.

    //TODO: Implement.
}
LevelEditor::~LevelEditor(void)
{
    //TODO: Implement.
}


void LevelEditor::Update(Vector2i mousePos, float frameSeconds)
{
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
                //TODO: Set up context menu. Make a class that inherits from GUISelectionBox to handle all context menu stuff. Probably will need an "OnContextMenu_[Option]()" callback for most choices.
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

        //TODO: Implement the "ES_CONTEXT_MENU" and "ES_CREATING_ROOM" cases.
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
    //TODO: Re-position the GUI.
}
void LevelEditor::OnCloseWindow(void)
{
    //TODO: Ask if the player is sure he wants to quit.
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
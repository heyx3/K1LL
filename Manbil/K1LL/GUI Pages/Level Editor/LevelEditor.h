#pragma once

#include "../Page.h"
#include "../../Level Info/LevelInfo.h"

#include "ContextMenu.h"
#include "GUIEditorGrid.h"
#include "GUIRoomSelection.h"
#include "GUILevelPathing.h"


//TODO: When saving a level, also save a thumbnail of it.
//TODO: Add a "Room Editor" state.
//TODO: Draw something on top of the team bases. Maybe just draw the spawn items in those rooms.

//The "level editor" page.
//Left-click to move rooms around and right-click to pan or to bring up a context menu.
class LevelEditor : public Page
{
public:

    LevelInfo LevelData;


    //If something went wrong, a message is output into the given string.
    LevelEditor(const std::string& levelFileName, PageManager* manager, std::string& outErrorMsg);
    ~LevelEditor(void);


    virtual void Update(Vector2i mousePos, float frameSeconds) override;
    virtual void Render(float frameSeconds) override;

    virtual void OnWindowResized(void) override;
    virtual void OnCloseWindow(void) override;

    virtual void OnOtherWindowEvent(sf::Event& windowEvent) override;


    void RenderRoom(const LevelInfo::RoomData& room, bool isPlacing,
                    Vector4f blendCol, float depth, float frameSeconds, const RenderInfo& info);

    //Callbacks for various UI events.
    void OnRoomSelection(unsigned int roomIndex);
    void OnButton_CreateRoom(void),
         OnButton_MoveRoom(void),
         OnButton_DeleteRoom(void),
         OnButton_PlaceTeam1(void),
         OnButton_PlaceTeam2(void),
         OnButton_SetSpawn(ItemTypes type),
         OnButton_Save(void),
         OnButton_Test(void),
         OnButton_Quit(void);

    Vector2f WorldPosToScreen(Vector2f worldPos) const;
    Vector2f WorldSizeToScreen(Vector2f worldSize) const;
    Vector2f ScreenPosToWorld(Vector2f screenPos) const;
    Vector2f ScreenSizeToWorld(Vector2f screenSize) const;


private:

    //The different states this editor can be in.
    enum EditorStates
    {
        //Not doing anything.
        ES_IDLE,
        //The player is placing a room after creating one or picking one up off the level grid.
        ES_PLACING_ROOM,
        //The player right-clicked, and we're waiting to see whether he releases or drags the mouse.
        ES_WAITING_ON_RIGHT_MOUSE,
        //Dragging the right mouse, which pans the camera.
        ES_PANNING,
        //Selecting something from the pop-up context menu, which appears with a right-click.
        ES_CONTEXT_MENU,
        //Selecting a new room to create.
        ES_CREATING_ROOM,
    };


    //The full relative path to the level file being edited.
    std::string levelFile;

    EditorStates currentState = ES_IDLE;
    Box2D worldViewBounds;

    Vector2f windowSizeF;

    //Don't count left mouse events as this page is starting up.
    bool ignoreLeftMouse = true;

    Vector2f currentMouseWorldPos;
    Vector2u currentMouseGridPos;

    ContextMenu contextMenu;
    GUIEditorGrid worldViewGrid;
    GUIRoomSelection newRoomSelection;
    GUILevelPathing levelPathing;
    
    //Whether the left mouse button was being pressed last frame.
    bool mouseLastFrame = false;

    //State-specific variables.
    LevelInfo::RoomData placingRoom_Room;
    bool placingRoom_IsPlacing; //Keeps track of this fact between multiple states.
    Vector2i waitingOnRightMouse_StartPos;
    Vector2i panning_LastPos;
    unsigned int contextMenu_SelectedRoom; //The room that was right-clicked on.
};
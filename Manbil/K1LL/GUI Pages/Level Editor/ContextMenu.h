#pragma once

#include "../../../Rendering/GUI/GUI Elements/GUISelectionBox.h"


class LevelEditor;

//The popup menu when the player right-clicks in the level editor.
class ContextMenu : public GUISelectionBox
{
public:

    //The different potential options on the context menu.
    enum Options
    {
        O_CREATE_ROOM = 0,
        O_MOVE_ROOM,
        O_DELETE_ROOM,
        O_PLACE_TEAM1,
        O_PLACE_TEAM2,
        O_SET_LIGHT_AMMO,
        O_SET_HEAVY_AMMO,
        O_SET_SPECIAL_AMMO,
        O_SET_HEALTH,
        O_SET_NONE,
        O_SAVE,
        O_TEST,
        O_QUIT,

        O_NUMBER_OF_ELEMENTS,
    };


    LevelEditor* Editor;
    void(*OnElementClicked)(Options element, ContextMenu* thisMenu);


    ContextMenu(LevelEditor* editor, std::string& outErrorMsg,
                void(*onElementClicked)(Options element, ContextMenu* thisMenu));


    //Sets up this menu to display the options for clicking on empty space.
    void SetUpForEmptySpace(void);
    //Sets up this menu to display the options for clicking on the given room.
    void SetUpForRoom(unsigned int roomIndex, bool onEmptySpace);
};
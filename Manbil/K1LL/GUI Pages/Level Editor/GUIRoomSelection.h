#pragma once

#include "../../../Rendering/GUI/GUI Elements/GUIPanel.h"

#include "../../Room Editor/RoomCollection.h"


class GUIEditorGrid;
class LevelEditor;
class GUIRoom;


//A selection of rooms the player can use when building a level.
class GUIRoomSelection : public GUIPanel
{
public:


    GUIRoomSelection(const GUIEditorGrid& grid, LevelEditor& editor, std::string& outErrorMsg);


    const LevelEditor& GetLevelEditor(void) const { return editor; }
    const RoomCollection& GetRooms(void) const { return rooms; }
    
    const GUIRoom& GetRoomElement(unsigned int index) const;

    //Call this when the window has been resized.
    void OnWindowResized(void);


private:

    LevelEditor& editor;
    RoomCollection rooms;
};
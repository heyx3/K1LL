#pragma once

#include "../../../Rendering/GUI/GUI Elements/GUIPanel.h"

#include "../../Room Editor/RoomCollection.h"
#include "../../Level Info/LevelInfo.h"


class GUIEditorGrid;
class LevelEditor;
class GUIRoom;


//A selection of rooms the player can use when building a level.
class GUIRoomSelection : public GUIPanel
{
public:


    GUIRoomSelection(const GUIEditorGrid& grid, LevelEditor& editor, std::string& outErrorMsg);


    const LevelEditor& GetLevelEditor(void) const { return editor; }
    const std::vector<LevelInfo::RoomData>& GetRooms(void) const { return roomDatas; }
    
    const GUIRoom& GetRoomElement(unsigned int index) const;

    //Call this when the window has been resized.
    void OnWindowResized(void);


private:

    LevelEditor& editor;

    RoomCollection rooms;
    std::vector<LevelInfo::RoomData> roomDatas;
};
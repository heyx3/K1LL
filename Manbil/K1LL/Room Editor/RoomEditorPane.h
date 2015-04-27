#pragma once

#include "../../Editor/IEditable.h"
#include "RoomCollection.h"

class GUILabel;


//Provides the editor pane and editable data for the Room editor.
struct RoomEditorPane : public IEditable
{
public:

    //The type of block being placed down.
    BlockTypes TypeBeingPlaced = BT_NONE;

    //The rooms being edited.
    RoomCollection Rooms;
    //The index of the current room being edited.
    unsigned int CurrentRoom = 0;
    

    virtual ~RoomEditorPane(void) { }


    //Saves this editor's data to the correct file.
    //If "true" is passed in, it will also save to the same file in the "Dependencies/..." folder.
    std::string SaveData(bool alsoSaveToDependencies) const;
    //Loads this editor's Room data from the correct file.
    std::string LoadData(void);

    RoomInfo& GetCurrentRoom(void) { return Rooms.Rooms[CurrentRoom]; }
    const RoomInfo& GetCurrentRoom(void) const { return Rooms.Rooms[CurrentRoom]; }

    const GUILabel* GetCurrentRoomLabel(void) const { return currentRoomLabel; }
    GUILabel* GetCurrentRoomLabel(void) { return currentRoomLabel; }

    void UpdateCurrentRoomLabel(void);

    virtual std::string BuildEditorElements(std::vector<EditorObjectPtr>& outElements,
                                            EditorMaterialSet& materialSet) override;

protected:

    unsigned int newGridX = 5,
                 newGridY = 5;

private:

    GUILabel* currentRoomLabel = 0;
};
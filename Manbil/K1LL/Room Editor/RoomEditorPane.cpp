#include "RoomEditorPane.h"

#include <iostream>

#include "../../Editor/EditorObjects.h"
#include "../../IO/BinarySerialization.h"


const std::string roomsFile = "Content/Rooms.bin";
const bool useBinaryErrorChecking = true;

std::string RoomEditorPane::SaveData(bool alsoSaveToDependencies) const
{
    BinaryWriter writer(useBinaryErrorChecking, Rooms.Rooms.size() * sizeof(RoomInfo));
    writer.WriteDataStructure(Rooms, "Room Collection");
    
    std::string err = writer.SaveData(roomsFile);
    if (!err.empty())
    {
        return "Error saving file to '" + roomsFile + "': " + err;
    }

    if (alsoSaveToDependencies)
    {
        err = writer.SaveData("../../Dependencies/Include In Build/Universal/" + roomsFile);
        if (!err.empty())
        {
            return "Error saving file '" + roomsFile + "' to 'Dependencies' folder: " + err;
        }
    }

    return "";
}
std::string RoomEditorPane::LoadData(void)
{
    BinaryReader reader(useBinaryErrorChecking, roomsFile);
    if (!reader.ErrorMessage.empty())
    {
        return "Error opening file '" + roomsFile + "': " + reader.ErrorMessage;
    }

    try
    {
        reader.ReadDataStructure(Rooms);
    }
    catch (int e)
    {
        assert(e == DataReader::EXCEPTION_FAILURE);
        return "Error reading data from file: " + reader.ErrorMessage;
    }

    return "";
}

std::string RoomEditorPane::BuildEditorElements(std::vector<EditorObjectPtr>& outElements,
                                            EditorMaterialSet& materialSet)
{
    std::vector<std::string> blockTypeValues;
    blockTypeValues.push_back("Empty");
    blockTypeValues.push_back("Wall");
    blockTypeValues.push_back("Doorway");
    blockTypeValues.push_back("Player Spawn");
    blockTypeValues.push_back("Item Spawn");
    auto onBlockSelected = [](GUISelectionBox* dropdownBox, const std::string& item,
                              unsigned int index, void* pData)
    {
        RoomEditorPane* editor = (RoomEditorPane*)pData;
        if (item == "Empty") {
            editor->TypeBeingPlaced = BT_NONE;
        } else if (item == "Wall") {
            editor->TypeBeingPlaced = BT_WALL;
        } else if (item == "Doorway") {
            editor->TypeBeingPlaced = BT_DOORWAY;
        } else if (item == "Player Spawn") {
            editor->TypeBeingPlaced = BT_PLAYER_SPAWN;
        } else if (item == "Item Spawn") {
            editor->TypeBeingPlaced = BT_ITEM_SPAWN;
        } else {
            assert(false);
        }
    };
    DropdownValues* blockTypes = new DropdownValues(blockTypeValues, Vector2f(0.0f, 15.0f),
                                                    EditorObject::DescriptionData("Block Type:", true, 30.0f, 512),
                                                    256, onBlockSelected, this);
    blockTypes->ItemBackgroundScale = Vector2f(0.378f, 0.5f);

    
    typedef TextBoxUInt<unsigned int*> MyTBUI;
    auto onGridValChanged = [](GUITextBox* box, unsigned int newVal, unsigned int* pData)
    {
        *pData = newVal;
    };
    MyTBUI *gridX = new MyTBUI(GetCurrentRoom().RoomGrid.GetWidth(), 50.0f, Vector2f(0.0f, 5.0f),
                               EditorObject::DescriptionData("New Grid Width"),
                               onGridValChanged, &newGridX),
           *gridY = new MyTBUI(GetCurrentRoom().RoomGrid.GetHeight(), 50.0f, Vector2f(0.0f, 5.0f),
                               EditorObject::DescriptionData("New Grid Height"),
                               onGridValChanged, &newGridY);

    struct ResizeRoomData
    {
        RoomEditorPane* Editor;
        unsigned int *NewGridX, *NewGridY;
        ResizeRoomData(RoomEditorPane* editor, unsigned int *newGridX, unsigned int* newGridY)
            : Editor(editor), NewGridX(newGridX), NewGridY(newGridY) { }
    };
    ResizeRoomData resizeData(this, &newGridX, &newGridY);

    auto OnResizeButtonClicked = [](GUITexture* buttonTex, Vector2f mouse, ResizeRoomData data)
    {
        RoomInfo& rm = data.Editor->GetCurrentRoom();
        rm.RoomGrid.Resize(*data.NewGridX, *data.NewGridY, BT_NONE);
        rm.RoomGrid.Fill(BT_NONE);
        for (unsigned int x = 0; x < *data.NewGridX; ++x)
        {
            rm.RoomGrid[Vector2u(x, 0)] = BT_WALL;
            rm.RoomGrid[Vector2u(x, rm.RoomGrid.GetHeight() - 1)] = BT_WALL;
        }
        for (unsigned int y = 0; y < *data.NewGridY; ++y)
        {
            rm.RoomGrid[Vector2u(0, y)] = BT_WALL;
            rm.RoomGrid[Vector2u(rm.RoomGrid.GetWidth() - 1, y)] = BT_WALL;
        }
    };

    typedef EditorButton<ResizeRoomData> MyButton;
    MyButton* createGridBtn = new MyButton("Regenerate Grid", Vector2f(100.0f, 25.0f),
                                           OnResizeButtonClicked, resizeData, 0, Vector2f(0.0f, 15.0f));

    //The label continually tells this editor pane which label displays the current room.
    EditorLabel* currentRoom = new EditorLabel("Room " + std::to_string(CurrentRoom + 1),
                                               512, Vector2f(0.0f, 50.0f));
    currentRoom->OnUpdate = [](GUILabel* label, float elapsed, void* pData)
    {
        *(GUILabel**)pData = label;
    };
    currentRoom->OnUpdate_pData = &currentRoomLabel;
    

    std::vector<EditorButtonData> buttons;
    buttons.push_back(EditorButtonData("<", 0, Vector2f(0.1f, 0.09f),
                                       [](GUITexture* clicked, Vector2f mouse, void* pData)
                                       {
                                           RoomEditorPane& p = *(RoomEditorPane*)pData;
                                           if (p.CurrentRoom == 0)
                                           {
                                               p.CurrentRoom = p.Rooms.Rooms.size() - 1;
                                           }
                                           else
                                           {
                                               p.CurrentRoom -= 1;
                                           }
                                           
                                           p.UpdateCurrentRoomLabel();
                                       }, this));
    buttons.push_back(EditorButtonData(">", 0, Vector2f(0.1f, 0.09f),
                                       [](GUITexture* clicked, Vector2f mouse, void* pData)
                                       {
                                           RoomEditorPane& p = *(RoomEditorPane*)pData;
                                           p.CurrentRoom = (p.CurrentRoom + 1) % p.Rooms.Rooms.size();
                                           
                                           p.UpdateCurrentRoomLabel();
                                       }, this));
    EditorButtonList* changeRoomButtons = new EditorButtonList(buttons, EditorObject::DescriptionData(),
                                                               5.0f, Vector2f(0.0f, 6.0f));

    buttons.clear();
    buttons.push_back(EditorButtonData("-", 0, Vector2f(0.1f, 0.09f),
                                       [](GUITexture* clicked, Vector2f mouse, void* pData)
                                       {
                                           RoomEditorPane& p = *(RoomEditorPane*)pData;
                                           if (p.Rooms.Rooms.size() == 1)
                                           {
                                               return;
                                           }

                                           p.Rooms.Rooms.erase(p.Rooms.Rooms.begin() + p.CurrentRoom);
                                           if (p.CurrentRoom == p.Rooms.Rooms.size())
                                           {
                                               p.CurrentRoom -= 1;

                                           p.UpdateCurrentRoomLabel();
                                           }
                                       }, this));
    buttons.push_back(EditorButtonData("+", 0, Vector2f(0.1f, 0.09f),
                                       [](GUITexture* clicked, Vector2f mouse, void* pData)
                                       {
                                           RoomEditorPane& p = *(RoomEditorPane*)pData;
                                           p.Rooms.Rooms.insert(p.Rooms.Rooms.begin() + p.CurrentRoom + 1,
                                                                RoomInfo());
                                           p.CurrentRoom += 1;
                                           
                                           p.UpdateCurrentRoomLabel();
                                       }, this));
    EditorButtonList* addRemoveButtons = new EditorButtonList(buttons, EditorObject::DescriptionData(),
                                                              5.0f, Vector2f(0.0f, 20.0f));

    
    auto onSaveButton = [](GUITexture* tex, Vector2f mouse, RoomEditorPane* pData)
    {
        std::string err = pData->SaveData(true);
        if (!err.empty())
        {
            std::cout << "Error saving data: " << err << "\n";
        }
    };
    typedef EditorButton<RoomEditorPane*> SaveButtonType;
    SaveButtonType* saveButton = new SaveButtonType("Save", Vector2f(100.0f, 25.0f),
                                                    onSaveButton, this);


    outElements.push_back(EditorObjectPtr(blockTypes));
    outElements.push_back(EditorObjectPtr(gridX));
    outElements.push_back(EditorObjectPtr(gridY));
    outElements.push_back(EditorObjectPtr(createGridBtn));
    outElements.push_back(EditorObjectPtr(currentRoom));
    outElements.push_back(EditorObjectPtr(changeRoomButtons));
    outElements.push_back(EditorObjectPtr(addRemoveButtons));
    outElements.push_back(EditorObjectPtr(saveButton));

    return "";
}

void RoomEditorPane::UpdateCurrentRoomLabel(void)
{
    std::string roomVal = "Room " + std::to_string(CurrentRoom + 1);
    bool tryL = currentRoomLabel->SetText(roomVal);
    assert(tryL);
}
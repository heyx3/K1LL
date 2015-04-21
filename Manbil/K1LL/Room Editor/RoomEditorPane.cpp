#include "RoomEditorPane.h"

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
        return "Error opening file: " + reader.ErrorMessage;
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
    DropdownValues* blockTypes = new DropdownValues(blockTypeValues, Vector2f(),
                                                    EditorObject::DescriptionData("Block Type"),
                                                    256, onBlockSelected, this);


    std::vector<std::string> placeTypeValues;
    placeTypeValues.push_back("Add Blocks");
    placeTypeValues.push_back("Add Nodes");
    placeTypeValues.push_back("Remove Nodes");
    auto onModeSelected = [](GUISelectionBox* dropdownBox, const std::string& item,
                             unsigned int index, void* pData)
    {
        RoomEditorPane* editor = (RoomEditorPane*)pData;
        if (item == "Add Blocks") {
            editor->PlacingStatus = PS_BLOCK;
        } else if (item == "Add Nodes") {
            editor->PlacingStatus = PS_NODE_ADD;
        } else if (item == "Remove Nodes") {
            editor->PlacingStatus = PS_NODE_REMOVE;
        }
    };
    DropdownValues* placeModes = new DropdownValues(placeTypeValues, Vector2f(),
                                                    EditorObject::DescriptionData("Placement Mode"),
                                                    256, onModeSelected, this);
    
    typedef TextBoxUInt<unsigned int*> MyTBUI;
    auto onGridValChanged = [](GUITextBox* box, unsigned int newVal, unsigned int* pData)
    {
        *pData = newVal;
    };
    MyTBUI *gridX = new MyTBUI(GetCurrentRoom().RoomGrid.GetWidth(), 50.0f, Vector2f(),
                               EditorObject::DescriptionData("New Grid Width"),
                               onGridValChanged, &newGridX),
           *gridY = new MyTBUI(GetCurrentRoom().RoomGrid.GetHeight(), 50.0f, Vector2f(),
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
            rm.RoomGrid[Vector2u(x, rm.RoomGrid.GetHeight())] = BT_WALL;
        }
        for (unsigned int y = 0; y < *data.NewGridY; ++y)
        {
            rm.RoomGrid[Vector2u(0, y)] = BT_WALL;
            rm.RoomGrid[Vector2u(rm.RoomGrid.GetWidth(), y)] = BT_WALL;
        }
    };

    typedef EditorButton<ResizeRoomData> MyButton;
    MyButton* createGridBtn = new MyButton("Regenerate Grid", Vector2f(200.0f, 50.0f),
                                           OnResizeButtonClicked, resizeData, 0);

    EditorLabel* currentRoom = new EditorLabel("Room ", 512);
    currentRoom->OnUpdate = [](GUILabel* label, float elapsed, void* pData)
    {
        bool tryL = label->SetText("Room " + std::to_string(((RoomEditorPane*)pData)->CurrentRoom));
        assert(tryL);
    };
    currentRoom->OnUpdate_pData = this;
    

    std::vector<EditorButtonData> buttons;
    buttons.push_back(EditorButtonData("<", 0, Vector2f(1.0f, 1.0f),
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
                                       }, this));
    buttons.push_back(EditorButtonData(">", 0, Vector2f(1.0f, 1.0f),
                                       [](GUITexture* clicked, Vector2f mouse, void* pData)
                                       {
                                           RoomEditorPane& p = *(RoomEditorPane*)pData;
                                           p.CurrentRoom = (p.CurrentRoom + 1) % p.Rooms.Rooms.size();
                                       }, this));
    EditorButtonList* changeRoomButtons = new EditorButtonList(buttons, EditorObject::DescriptionData(),
                                                               5.0f);


    outElements.push_back(EditorObjectPtr(blockTypes));
    outElements.push_back(EditorObjectPtr(placeModes));
    outElements.push_back(EditorObjectPtr(gridX));
    outElements.push_back(EditorObjectPtr(gridY));
    outElements.push_back(EditorObjectPtr(createGridBtn));
    outElements.push_back(EditorObjectPtr(currentRoom));
    outElements.push_back(EditorObjectPtr(changeRoomButtons));
    

    return "";
}
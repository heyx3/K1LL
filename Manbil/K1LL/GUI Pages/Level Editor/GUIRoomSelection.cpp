#include "GUIRoomSelection.h"

#include "../../../IO/BinarySerialization.h"

#include "../../Content/MenuContent.h"
#include "LevelEditor.h"
#include "GUIRoom.h"
#include "../PageManager.h"


namespace
{
    const std::string roomsFile = "Content/Rooms.bin";

    const float DEPTH_Floor = 0.8f;


    struct InputData
    {
        const GUIEditorGrid* Grid;
        LevelEditor* Editor;
    };

    GUIElementPtr CreateRoom(InputData& data, unsigned int roomIndex, const RoomInfo* roomData)
    {
        GUIElementPtr ptr(new GUIRoom(*data.Grid, *data.Editor, false, roomData, IT_NONE));
        ptr->Depth = DEPTH_Floor + 0.0001f;
        ptr->SetColor(Vector4f(2.0f, 2.0f, 2.0f, 1.0f));
        
        ((GUIRoom*)ptr.get())->IsButton = true;
        ((GUIRoom*)ptr.get())->TimeLerpSpeed = 3.0f;

        ((GUIRoom*)ptr.get())->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
        {
            LevelEditor& edit = ((GUIRoom*)tex)->Editor;
            edit.OnRoomSelection((unsigned int)pData);
        };
        ((GUIRoom*)ptr.get())->OnClicked_pData = (void*)roomIndex;

        return ptr;
    }
}

GUIRoomSelection::GUIRoomSelection(const GUIEditorGrid& grid, LevelEditor& _editor, std::string& err)
    : editor(_editor),
      GUIPanel(GUITexture(MenuContent::Instance.StaticColorGUIParams, 0,
                          MenuContent::Instance.StaticColorGUIMat),
                Vector2f(9999.0f, 9999.0f))
{
    //Read in the room files.
    BinaryReader reader(true, roomsFile);
    if (!reader.ErrorMessage.empty())
    {
        err = "Error opening file '" + roomsFile + "': " + reader.ErrorMessage;
        return;
    }
    try
    {
        reader.ReadDataStructure(rooms);
    }
    catch (int e)
    {
        assert(e == DataReader::EXCEPTION_FAILURE);
        err = "Error reading data from file: " + reader.ErrorMessage;
        return;
    }

    //Background.SetColor(Vector4f(0.0f, 0.0f, 0.0f, 0.9f));
    Background.Depth = DEPTH_Floor;

    //Create each of the rooms.
    InputData dat;
    dat.Editor = &editor;
    dat.Grid = &grid;
    for (unsigned int i = 0; i < rooms.Rooms.size(); ++i)
    {
        AddElement(CreateRoom(dat, i, &rooms.Rooms[i]));
    }

    //Re-position the rooms.
    OnWindowResized();
}

const GUIRoom& GUIRoomSelection::GetRoomElement(unsigned int index) const
{
    return *(GUIRoom*)(GetElements()[index].get());
}

void GUIRoomSelection::OnWindowResized(void)
{
    Vector2f windowSize = ToV2f(editor.Manager->GetWindowSize());

    //Calculate spacing information for placing the choices.
    unsigned int choicesPerRow = 5;
    float choiceBorder = 10.0f;
    float choiceSize = (windowSize.x - (choiceBorder * (choicesPerRow + 1))) /
                       (float)choicesPerRow;
    const float maxSize = 300.0f;
    while (choiceSize > maxSize)
    {
        choicesPerRow += 1;
        choiceSize = (windowSize.x - (choiceBorder * (choicesPerRow + 1))) /
                     (float)choicesPerRow;
    }
    
    //Make sure the choices aren't so big that they go off the bottom of the screen.
    const unsigned int choicesPerColumn = (rooms.Rooms.size() / choicesPerRow) +
                                          (rooms.Rooms.size() % choicesPerRow > 0 ?
                                               1 :
                                               0);
    float totalHeight = choiceBorder + choiceBorder +
                        ((float)choicesPerColumn * (choiceBorder + choiceSize));
    float extraHeight = totalHeight - windowSize.y;
    if (extraHeight > 0.0f)
    {
        choiceSize *= (totalHeight - extraHeight) / totalHeight;
    }

    //Pre-calculate the x positions on each row.
    assert(128 >= choicesPerRow);
    float xPoses[128];
    for (unsigned int i = 0; i < choicesPerRow; ++i)
    {
        xPoses[i] = choiceBorder + ((float)i * (choiceSize + choiceBorder));
    }

    //Position each of the rooms.
    float yPos = -windowSize.y + choiceBorder;
    for (unsigned int i = 0; i < rooms.Rooms.size(); ++i)
    {
        Box2D fullBounds(xPoses[i % choicesPerRow], yPos,
                         Vector2f(choiceSize, choiceSize));

        //Scale each element's bounds to give it the correct aspect ratio.
        const Array2D<BlockTypes>& grid = rooms.Rooms[i].RoomGrid;
        if (grid.GetWidth() > grid.GetHeight())
        {
            fullBounds.Scale(Vector2f(1.0f, (float)grid.GetHeight() / (float)grid.GetWidth()));
        }
        else if (grid.GetHeight() > grid.GetWidth())
        {
            fullBounds.Scale(Vector2f((float)grid.GetWidth() / (float)grid.GetHeight(), 1.0f));
        }

        GetElements()[i]->SetBounds(fullBounds);

        if ((i + 1) % choicesPerRow == 0)
        {
            yPos += choiceSize + choiceBorder;
        }
    }
}
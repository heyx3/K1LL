#include "ContextMenu.h"

#include "../../Content/MenuContent.h"
#include "LevelEditor.h"


#define MC MenuContent::Instance


namespace
{
    //Scaling constants.
    const Vector2f SCALE_Item(0.5f, 0.25f),
                   SCALE_Highlight(0.25f, 0.3f),
                   SCALE_Total(2.3f, 2.3f),
                   SCALE_Text(1.4f, 1.4f);

    //Color constants.
    const Vector4f COLOR_Text(1.0f, 1.0f, 1.0f, 1.0f),
                   COLOR_Background(0.05f, 0.05f, 0.05f, 1.0f),
                   COLOR_Highlight(0.2f, 0.2f, 2.0f, 1.0f),
                   COLOR_Total(1.0f, 1.0f, 1.0f, 0.95f);

    //Spacing constants.
    const float SPACING_Items = 5.0f;


#define OP ContextMenu::Options

    std::string ToString(OP opt)
    {
        switch (opt)
        {
            case OP::O_CREATE_ROOM: return "Create room";
            case OP::O_MOVE_ROOM: return "Move room";
            case OP::O_DELETE_ROOM: return "Delete room";
            case OP::O_PLACE_TEAM1: return "Place Team 1 here";
            case OP::O_PLACE_TEAM2: return "Place Team 2 here";
            case OP::O_SET_LIGHT_AMMO: return "Room has light ammo";
            case OP::O_SET_HEAVY_AMMO: return "Room has heavy ammo";
            case OP::O_SET_SPECIAL_AMMO: return "Room has special ammo";
            case OP::O_SET_LIGHT_WEAPON: return "Room has light weapon";
            case OP::O_SET_HEAVY_WEAPON: return "Room has heavy weapon";
            case OP::O_SET_SPECIAL_WEAPON: return "Room has special weapon";
            case OP::O_SET_HEALTH: return "Room has health";
            case OP::O_SET_NONE: return "Room has nothing";
            case OP::O_SAVE: return "Save level";
            case OP::O_TEST: return "Test level";
            case OP::O_QUIT: return "Quit editor";

            default:
                assert(false);
                return "UNKNOWN_VALUE_" + std::to_string(opt);
        }
    }

    std::vector<std::string> GenerateItems(void)
    {
        std::vector<std::string> vals;
        vals.resize((unsigned int)OP::O_NUMBER_OF_ELEMENTS);

        //Generate the items in the order they were defined in.
        for (unsigned int i = 0; i < (unsigned int)ContextMenu::O_NUMBER_OF_ELEMENTS; ++i)
        {
            vals[i] = ToString((OP)i);
        }

        return vals;
    }

#undef OP
}

ContextMenu::ContextMenu(LevelEditor* editor, std::string& err,
                         void(*onElementClicked)(Options element, ContextMenu* thisMenu))
    : Editor(editor), OnElementClicked(onElementClicked),
      GUISelectionBox(&MC.TextRender, MC.MainTextFont, COLOR_Text, false, FT_LINEAR,
                      MC.MainTextFontScale.ComponentProduct(SCALE_Text), SPACING_Items,
                      MC.LabelGUIMat, MC.LabelGUIParams,
                      MC.CreateGUITexture(&MC.TextBoxBackground, true),
                      MC.CreateGUITexture(&MC.LevelSelectionBoxHighlight, false),
                      MC.CreateGUITexture(&MC.TextBoxBackground, false),
                      false, err, GenerateItems(),
                      [](GUISelectionBox* box, const std::string& item, unsigned int i, void* pData)
                      {
                          ContextMenu* mnu = (ContextMenu*)pData;
                          mnu->OnElementClicked((ContextMenu::Options)i, mnu);
                      },
                      0, MC.MainTextFontHeight, this)
{
    if (err.empty())
    {
        MainBox.ScaleBy(SCALE_Item);
        Highlight.ScaleBy(SCALE_Highlight);
        ScaleBy(SCALE_Total);

        Highlight.SetColor(COLOR_Highlight);
        SelectionBackground.SetColor(COLOR_Background);
        SetColor(COLOR_Total);
    }
}

void ContextMenu::SetUpForEmptySpace(void)
{
    for (unsigned int i = 0; i < GetItems().size(); ++i)
    {
        switch ((Options)i)
        {
            #define CASE(option, shouldBeShown) \
                case Options::O_ ## option : \
                    SetIsItemHidden(i, !shouldBeShown); \
                    break;
            

            CASE(CREATE_ROOM, true)
            CASE(MOVE_ROOM, false)
            CASE(DELETE_ROOM, false)
            CASE(PLACE_TEAM1, false)
            CASE(PLACE_TEAM2, false)
            CASE(SET_LIGHT_AMMO, false)
            CASE(SET_HEAVY_AMMO, false)
            CASE(SET_SPECIAL_AMMO, false)
            CASE(SET_LIGHT_WEAPON, false)
            CASE(SET_HEAVY_WEAPON, false)
            CASE(SET_SPECIAL_WEAPON, false)
            CASE(SET_HEALTH, false)
            CASE(SET_NONE, false)
            CASE(SAVE, true)
            CASE(TEST, true)
            CASE(QUIT, true)

            default:
                assert(false);
                break;


            #undef CASE
        }
    }
}
void ContextMenu::SetUpForRoom(unsigned int roomIndex)
{
    bool hasSpawns = Editor->LevelData.Rooms[roomIndex].GetRoomHasSpawns();

    for (unsigned int i = 0; i < GetItems().size(); ++i)
    {
        switch ((Options)i)
        {
            #define CASE(option, shouldBeShown) \
                case Options::O_ ## option : \
                    SetIsItemHidden(i, !shouldBeShown); \
                    break;
            

            CASE(CREATE_ROOM, true)
            CASE(MOVE_ROOM, true)
            CASE(DELETE_ROOM, true)
            CASE(PLACE_TEAM1, true)
            CASE(PLACE_TEAM2, true)
            CASE(SET_LIGHT_AMMO, hasSpawns)
            CASE(SET_HEAVY_AMMO, hasSpawns)
            CASE(SET_SPECIAL_AMMO, hasSpawns)
            CASE(SET_LIGHT_WEAPON, hasSpawns)
            CASE(SET_HEAVY_WEAPON, hasSpawns)
            CASE(SET_SPECIAL_WEAPON, hasSpawns)
            CASE(SET_HEALTH, hasSpawns)
            CASE(SET_NONE, hasSpawns)
            CASE(SAVE, true)
            CASE(TEST, true)
            CASE(QUIT, true)

            default:
                assert(false);
                break;


            #undef CASE
        }
    }
}
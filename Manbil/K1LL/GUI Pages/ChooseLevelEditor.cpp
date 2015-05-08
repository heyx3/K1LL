#include "ChooseLevelEditor.h"

#include "../../DebugAssist.h"
#include "../tinydir.h"
#include "../../IO/BinarySerialization.h"

#include "../Level Info/LevelInfo.h"
#include "../Content/MenuContent.h"
#include "PageManager.h"
#include "MainMenu.h"
#include "LevelEditor.h"


#define MC MenuContent::Instance


ChooseLevelEditor::ChooseLevelEditor(PageManager* manager, std::string& err)
    : Page(manager)
{
    //Build out GUI stuff.

    #pragma region The "Enter new level name" panel

    //The panel.
    enterLevelName = GUIElementPtr(new GUIFormattedPanel(10.0f, 5.0f,
                                                         MC.CreateGUITexture(&MC.PageBackground,
                                                                             false)));


    //The textbox.
    GUITexture enterLevelTextboxBackground = MC.CreateGUITexture(&MC.TextBoxBackground, false),
               enterLevelTextboxCursor = MC.CreateGUITexture(0, false);
    enterLevelTextboxCursor.ScaleBy(Vector2f(3.0f, (float)MC.MainTextFontHeight * MC.MainTextFontScale.y));
    
    GUILabel enterLevelText = MC.CreateGUILabel(MC.MainTextFont, MC.MainTextFontHeight,
                                                MC.MainTextFontScale, 1024, err,
                                                GUILabel::HO_LEFT, GUILabel::VO_CENTER, 0, Vector3f());
    if (!err.empty())
    {
        err = "Error creating text box render slot for level name entry: " + err;
        return;
    }

    enterLevelTextBox = new GUITextBox(enterLevelTextboxBackground, enterLevelTextboxCursor,
                                       enterLevelText, true, 4.0f);
    enterLevelTextBox->Contents.DeleteSlotWhenDeleted = true;
    GetEnterLevelNamePanel()->AddObject(GUIFormatObject(GUIElementPtr(enterLevelTextBox), true, false,
                                                        Vector2f(20.0f, 0.0f)));


    //The "Create" button.
    GUITexture* createLevelButton = new GUITexture(MC.CreateGUITexture(&MC.EditLevelTex, true));
    createLevelButton->OnClicked = [](GUITexture* thisTex, Vector2f mousePos, void* pData)
    {
        ChooseLevelEditor* cle = (ChooseLevelEditor*)pData;

        std::string lvlName = cle->GenerateLevel();
        if (!lvlName.empty())
        {
            //TODO: Transition to level editor.
            assert(false);
            //cle->Manager->CurrentPage = Page::Ptr(new LevelEditor(lvlName, cle->Manager));
        }
    };
    createLevelButton->OnClicked_pData = this;
    GetEnterLevelNamePanel()->AddObject(GUIFormatObject(GUIElementPtr(createLevelButton), true, false,
                                                        Vector2f(10.0f, 0.0f)));


    //The "back" button.
    GUITexture* backToIdleButton = new GUITexture(MC.CreateGUITexture(&MC.BackButton, true));
    backToIdleButton->OnClicked = [](GUITexture* thisTex, Vector2f mousePos, void* pData)
    {
        ChooseLevelEditor* cle = (ChooseLevelEditor*)pData;
        cle->ChangeState(ChooseLevelEditor::PS_IDLE);
    };
    backToIdleButton->OnClicked_pData = this;
    GetEnterLevelNamePanel()->AddObject(GUIFormatObject(GUIElementPtr(backToIdleButton), true, false));

    #pragma endregion

    #pragma region The "Level options" panel
    
    //The panel.
    levelOptionsPanel = GUIElementPtr(new GUIPanel(MC.CreateGUITexture(&MC.PageBackground, false),
                                                   Vector2f(10.0f, 10.0f)));
    

    //The "delete" button.
    GUITexture* deleteLevelButton = new GUITexture(MC.CreateGUITexture(&MC.DeleteLevelTex, true));
    deleteLevelButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ((ChooseLevelEditor*)pData)->ChangeState(ChooseLevelEditor::PS_CONFIRM_DELETE);
    };
    deleteLevelButton->OnClicked_pData = this;
    deleteLevelButton->SetPosition(Vector2f(-50.0f, 20.0f));
    GetLevelOptionsPanel()->AddElement(GUIElementPtr(deleteLevelButton));


    //The "back" button. Just copy the other back button and save its tex.
    GUITexture* backToIdleButton2 = new GUITexture(*backToIdleButton);
    backToIdleButton2->SetPosition(Vector2f(50.0f, 20.0f));
    GetLevelOptionsPanel()->AddElement(GUIElementPtr(backToIdleButton2));


    #pragma endregion

    #pragma region The "Confirm delete level" panel

    //The panel.
    confirmDelete = GUIElementPtr(new GUIPanel(MC.CreateGUITexture(&MC.ConfirmDeletePopup, false),
                                               Vector2f(10.0f, 10.0f)));

    //The "YES" button.
    GUITexture* confirmDeleteButton = new GUITexture(MC.CreateGUITexture(&MC.YESTex, true));
    confirmDeleteButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ((ChooseLevelEditor*)pData)->DeleteLevel();
    };
    confirmDeleteButton->OnClicked_pData = this;
    confirmDeleteButton->SetPosition(Vector2f(-50.0f, 20.0f));
    GetConfirmDeletePanel()->AddElement(GUIElementPtr(confirmDeleteButton));

    //The "NO" button.
    GUITexture* cancelDeleteButton = new GUITexture(MC.CreateGUITexture(&MC.NOTex, true));
    cancelDeleteButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ((ChooseLevelEditor*)pData)->ChangeState(ChooseLevelEditor::PS_LEVEL_OPTIONS);
    };
    cancelDeleteButton->OnClicked_pData = this;
    cancelDeleteButton->SetPosition(Vector2f(50.0f, 20.0f));
    GetConfirmDeletePanel()->AddElement(GUIElementPtr(cancelDeleteButton));

    #pragma endregion

    #pragma region The idle "Choose level" panel

    //The panel.
    idlePanel = GUIElementPtr(new GUIPanel(MC.CreateGUITexture(&MC.PageBackground, false)));

    //The dropdown box.
    levelChoices = 0;
    DiscoverLevelFiles();

    //The "back" button.
    GUITexture* toMainMenuButton = new GUITexture(MC.CreateGUITexture(&MC.BackButton, true));
    toMainMenuButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        PageManager* mngr = (PageManager*)pData;
        mngr->CurrentPage = Page::Ptr(new MainMenu(mngr));
    };
    toMainMenuButton->OnClicked_pData = Manager;
    toMainMenuButton->SetPosition(Vector2f(300.0f, -300.0f));
    GetIdlePanel()->AddElement(GUIElementPtr(toMainMenuButton));

    //The "new level" button.
    GUITexture* newLevelButton = new GUITexture(MC.CreateGUITexture(&MC.CreateLevelTex, true));
    newLevelButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ChooseLevelEditor* pge = (ChooseLevelEditor*)pData;
        pge->ChangeState(ChooseLevelEditor::PS_ENTER_NEW_NAME);
    };
    newLevelButton->OnClicked_pData = this;
    newLevelButton->SetPosition(Vector2f(-300.0f, -300.0f));
    GetIdlePanel()->AddElement(GUIElementPtr(newLevelButton));

    #pragma endregion


    //Set up the level list and state.
    ChangeState(PS_IDLE);
}
ChooseLevelEditor::~ChooseLevelEditor(void)
{

}

void ChooseLevelEditor::Update(Vector2i mPos, float frameSeconds)
{
    Page::Update(mPos, frameSeconds);

    levelChoices->SetIsExtended(true, false);
}

void ChooseLevelEditor::OnWindowResized(void)
{
    RePositionGUI();
}
void ChooseLevelEditor::OnCloseWindow(void)
{
    Manager->EndWorld();
}

void ChooseLevelEditor::OnWindowGainedFocus(void)
{
    Page::OnWindowGainedFocus();
    DiscoverLevelFiles();
}

void ChooseLevelEditor::DiscoverLevelFiles(void)
{
    //Find the available levels.
    std::vector<std::string> items;

    tinydir_dir dir;
    tinydir_open(&dir, LevelInfo::LevelFilesPath.c_str());
    std::cout << "------------------------------\n";

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        std::cout << file.name << "\n";

        //Remove the extension from the name and add it to the list.
        std::string levelName = file.name;
        if (levelName.substr(levelName.size() - 4, 4) == ".lvl")
        {
            items.push_back(levelName.substr(0, levelName.size() - 4));
        }

        tinydir_next(&dir);
    }

    tinydir_close(&dir);
    std::cout << "-----------------------------\n\n\n";


    //Create or reset the dropdown box.
    std::string err;
    if (levelChoices == 0)
    {
        auto onOptionSelected = [](GUISelectionBox* box, const std::string& item, unsigned int i,
                                   void* pData)
        {
            ChooseLevelEditor* pge = (ChooseLevelEditor*)pData;
            if (pge->GetState() == ChooseLevelEditor::PS_IDLE)
            {
                pge->ChangeState(ChooseLevelEditor::PS_LEVEL_OPTIONS);
            }
        };
        levelChoices = new GUISelectionBox(&MC.TextRender, MC.MainTextFont,
                                           Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
                                           false, FT_LINEAR, Vector2f(1.0f, 1.0f), 10.0f,
                                           MC.LabelGUIMat, MC.LabelGUIParams,
                                           MC.CreateGUITexture(&MC.LevelSelectionBoxBackground, false),
                                           MC.CreateGUITexture(&MC.LevelSelectionBoxHighlight, false),
                                           MC.CreateGUITexture(&MC.LevelSelectionBoxBackground, false),
                                           true, err, items, onOptionSelected, 0, MC.MainTextFontHeight,
                                           this);
        if (!Assert(err.empty(), "Error creating level choice boxes", err))
        {
            delete levelChoices;
            return;
        }
    }
    else
    {
        levelChoices->ResetItems(items, err);
        if (!Assert(err.empty(), "Error creating new item boxes", err))
        {
            return;
        }
    }
}

std::string ChooseLevelEditor::GenerateLevel(void)
{
    std::string lvlName = enterLevelTextBox->GetText();

    //Make sure the level's name is valid.
    if (lvlName.length() == 0)
    {
        return "";
    }
    for (unsigned int i = 0; i < lvlName.length(); ++i)
    {
        //Valid chars are letters, numbers, hyphen, underscore, and space.
        char c = lvlName[i];
        if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') &&
            (c < '0' || c > '9') && c != '-' && c != '_' && c != ' ')
        {
            return "";
        }
    }
 
    //Create a blank level and write it out to the file.
    LevelInfo lvlData;
    BinaryWriter writer(true);
    writer.WriteDataStructure(lvlData, "Data");
    std::string fullLvlPath = LevelInfo::LevelFilesPath + lvlName + ".lvl";
    std::string err = writer.SaveData(fullLvlPath);
    if (!err.empty())
    {
        std::cout << "Error saving level to file '" + fullLvlPath + "': " + err;
        return "";
    }


    return lvlName;
}
void ChooseLevelEditor::DeleteLevel(void)
{
    std::string fileName = levelChoices->GetItems()[levelChoices->GetSelectedObject()];
    std::string fullPath = LevelInfo::LevelFilesPath + fileName + ".lvl";

    int error = std::remove(fullPath.c_str());
    if (!Assert(error == 0, "Error deleting file '" + fullPath, std::to_string(error)))
    {
        return;
    }
}

void ChooseLevelEditor::ChangeState(PageStates newState)
{
    std::string err;

    switch (newState)
    {
        case PS_IDLE:
            GUIManager.RootElement = idlePanel;
            break;

        case PS_ENTER_NEW_NAME:
            GUIManager.RootElement = enterLevelName;
            err = enterLevelTextBox->SetText("LevelNameHere");
            if (!Assert(err.empty(), "Error setting text for 'Enter Level Name' text box", err))
            {
                return;
            }
            break;

        case PS_LEVEL_OPTIONS:
            GUIManager.RootElement = levelOptionsPanel;
            break;

        case PS_CONFIRM_DELETE:
            GUIManager.RootElement = confirmDelete;
            break;

        default:
            assert(false);
            break;
    }
}
void ChooseLevelEditor::RePositionGUI(void)
{
    //TODO: Implement.
}
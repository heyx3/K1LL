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
    LevelInfo lvlData;
    BinaryWriter writer(true);
    writer.WriteDataStructure(lvlData, "Data");
    std::string fullLvlPath = LevelInfo::LevelFilesPath + "Test Level.lvl";
    err = writer.SaveData(fullLvlPath);
    if (!err.empty())
    {
        err = "Error saving level to file '" + fullLvlPath + "': " + err;
        return;
    }

    //Build out GUI stuff.

    #pragma region The "Enter new level name" panel

    //The panel.
    enterLevelName = GUIElementPtr(new GUIFormattedPanel(10.0f, 5.0f,
                                                         MC.CreateGUITexture(&MC.PageBackground,
                                                                             false)));


    //The textbox.
    GUITexture enterLevelTextboxBackground = MC.CreateGUITexture(&MC.TextBoxBackground, false),
               enterLevelTextboxCursor = MC.CreateGUITexture(0, false);
    enterLevelTextboxCursor.ScaleBy(Vector2f(3.0f, (float)MC.MainTextFontHeight * MC.MainTextFontScale.y * 0.75f));
    
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
    createLevelButton->Depth = 0.01f;
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
    backToIdleButton->Depth = 0.01f;
    GetEnterLevelNamePanel()->AddObject(GUIFormatObject(GUIElementPtr(backToIdleButton), true, false));

    #pragma endregion

    #pragma region The "Level options" panel
    
    //The panel.
    levelOptionsPanel = GUIElementPtr(new GUIPanel(MC.CreateGUITexture(&MC.PageBackground, false),
                                                   Vector2f(10.0f, 10.0f)));


    //The "edit" button.
    GUITexture* editLevelButton = new GUITexture(MC.CreateGUITexture(&MC.EditLevelTex, true));
    editLevelButton->OnClicked = [](GUITexture* thisTex, Vector2f mousePos, void* pData)
    {
        ChooseLevelEditor* cle = (ChooseLevelEditor*)pData;
        //TODO: Transition to level editor.
        assert(false);
        //cle->Manager->UpdateCurrentPage(Page::Ptr(new LevelEditor(cle->enterLevelTextBox->GetText())));
    };
    editLevelButton->OnClicked_pData = this;
    editLevelButton->Depth = 0.01f;
    editLevelButton->ScaleBy(Vector2f(0.75f, 0.75f));
    editLevelButton->SetPosition(Vector2f(-175.0f, 20.0f));
    GetLevelOptionsPanel()->AddElement(GUIElementPtr(editLevelButton));
    

    //The "delete" button.
    GUITexture* deleteLevelButton = new GUITexture(MC.CreateGUITexture(&MC.DeleteLevelTex, true));
    deleteLevelButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ((ChooseLevelEditor*)pData)->ChangeState(ChooseLevelEditor::PS_CONFIRM_DELETE);
    };
    deleteLevelButton->OnClicked_pData = this;
    deleteLevelButton->Depth = 0.01f;
    deleteLevelButton->ScaleBy(Vector2f(0.75f, 0.75f));
    deleteLevelButton->SetPosition(Vector2f(-65.0f, 20.0f));
    GetLevelOptionsPanel()->AddElement(GUIElementPtr(deleteLevelButton));


    //The "back" button. Just copy the other back button and save its texture.
    GUITexture* backToIdleButton2 = new GUITexture(*backToIdleButton);
    backToIdleButton2->ScaleBy(Vector2f(0.75f, 0.75f));
    backToIdleButton2->SetPosition(Vector2f(75.0f, 20.0f));
    GetLevelOptionsPanel()->AddElement(GUIElementPtr(backToIdleButton2));


    #pragma endregion

    #pragma region The "Confirm delete level" panel

    //The panel.
    confirmDelete = GUIElementPtr(new GUIPanel());

    //The background.
    GUITexture* confirmDeleteBack = new GUITexture(MC.CreateGUITexture(&MC.ConfirmDeletePopup, false));
    confirmDeleteBack->Depth = -0.01f;
    confirmDeleteBack->SetPosition(Vector2f(0.0f, 0.0f));
    confirmDeleteBack->SetScale(Vector2f(1.0f, 1.0f));
    GetConfirmDeletePanel()->AddElement(GUIElementPtr(confirmDeleteBack));

    //The "YES" button.
    GUITexture* confirmDeleteButton = new GUITexture(MC.CreateGUITexture(&MC.YESTex, true));
    confirmDeleteButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ((ChooseLevelEditor*)pData)->DeleteLevel();
        ((ChooseLevelEditor*)pData)->DiscoverLevelFiles();
        ((ChooseLevelEditor*)pData)->ChangeState(PS_IDLE);
    };
    confirmDeleteButton->OnClicked_pData = this;
    confirmDeleteButton->Depth = 0.01f;
    confirmDeleteButton->ScaleBy(Vector2f(1.0f, 1.0f));
    confirmDeleteButton->SetPosition(Vector2f(-100.0f, 25.0f));
    GetConfirmDeletePanel()->AddElement(GUIElementPtr(confirmDeleteButton));

    //The "NO" button.
    GUITexture* cancelDeleteButton = new GUITexture(MC.CreateGUITexture(&MC.NOTex, true));
    cancelDeleteButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ((ChooseLevelEditor*)pData)->ChangeState(ChooseLevelEditor::PS_LEVEL_OPTIONS);
    };
    cancelDeleteButton->OnClicked_pData = this;
    cancelDeleteButton->Depth = 0.01f;
    cancelDeleteButton->ScaleBy(Vector2f(1.0f, 1.0f));
    cancelDeleteButton->SetPosition(Vector2f(100.0f, 25.0f));
    GetConfirmDeletePanel()->AddElement(GUIElementPtr(cancelDeleteButton));

    #pragma endregion

    #pragma region The idle "Choose level" panel

    //The panel.
    idlePanel = GUIElementPtr(new GUIPanel());

    //The "back" button.
    mainBackButton = new GUITexture(MC.CreateGUITexture(&MC.BackButton, true));
    mainBackButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        PageManager* mngr = (PageManager*)pData;
        mngr->UpdateCurrentPage(Page::Ptr(new MainMenu(mngr)));
    };
    mainBackButton->OnClicked_pData = Manager;
    mainBackButton->Depth = 0.01f;
    GetIdlePanel()->AddElement(GUIElementPtr(mainBackButton));

    //The "new level" button.
    mainNewButton = new GUITexture(MC.CreateGUITexture(&MC.CreateLevelTex, true));
    mainNewButton->OnClicked = [](GUITexture* tex, Vector2f mousePos, void* pData)
    {
        ChooseLevelEditor* pge = (ChooseLevelEditor*)pData;
        pge->ChangeState(ChooseLevelEditor::PS_ENTER_NEW_NAME);
    };
    mainNewButton->OnClicked_pData = this;
    mainNewButton->Depth = 0.01f;
    GetIdlePanel()->AddElement(GUIElementPtr(mainNewButton));

    //The dropdown box.
    levelChoices = 0;
    DiscoverLevelFiles();
    GetIdlePanel()->AddElement(GUIElementPtr(levelChoices));

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
    tinydir_open(&dir, LevelInfo::LevelFilesPath.substr(0, LevelInfo::LevelFilesPath.size() - 1).c_str());
    std::cout << "------------------------------\n";

    while (dir.has_next)
    {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        //Remove the extension from the name and add it to the list.
        std::string levelName = file.name;
        if (levelName.size() > 4 && levelName.substr(levelName.size() - 4, 4) == ".lvl")
        {
            std::cout << file.name << "\n";
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
        GUITexture singleElementBackground = MC.CreateGUITexture(&MC.LevelSelectionSingleElement, false),
                   elementHighlight = MC.CreateGUITexture(&MC.LevelSelectionBoxHighlight, false);
        singleElementBackground.SetScale(Vector2f(300.0f, 20.0f));
        elementHighlight.SetBounds(Box2D(Vector2f(), singleElementBackground.GetBounds().GetDimensions().ComponentProduct(Vector2f(1.1f, 1.1f))));
        levelChoices = new GUISelectionBox(&MC.TextRender, MC.MainTextFont,
                                           Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
                                           false, FT_LINEAR, Vector2f(1.0f, 1.0f), 10.0f,
                                           MC.LabelGUIMat, MC.LabelGUIParams,
                                           singleElementBackground, elementHighlight,
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
        std::cout << "Error saving level to file '" << fullLvlPath << "': " << err;
        return "";
    }


    return lvlName;
}
void ChooseLevelEditor::DeleteLevel(void)
{
    std::string fileName = levelChoices->GetItems()[levelChoices->GetSelectedObject()];
    std::string fullPath = LevelInfo::LevelFilesPath + fileName + ".lvl";

    int error = std::remove(fullPath.c_str());
    if (!Assert(error == 0, "Error deleting file '" + fullPath + "'", std::to_string(error)))
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

    RePositionGUI();
}
void ChooseLevelEditor::RePositionGUI(void)
{
    Vector2f fullWindow = ToV2f(Manager->GetWindowSize()),
             halfWindow = fullWindow * 0.5f;

    enterLevelName->SetPosition(halfWindow.FlipY());
    levelOptionsPanel->SetPosition(halfWindow.FlipY());
    confirmDelete->SetPosition(halfWindow.FlipY());

    Vector2f backButtonSize = mainBackButton->GetBounds().GetDimensions(),
             newButtonSize = mainNewButton->GetBounds().GetDimensions();
    Vector2f backButtonPos(fullWindow.x - backButtonSize.x - 10.0f,
                           -fullWindow.y + backButtonSize.y + 10.0f);
    mainBackButton->SetPosition(backButtonPos);
    mainNewButton->SetPosition(Vector2f(backButtonPos.x - (backButtonSize.x * 0.5f) -
                                            10.0f - (newButtonSize.x * 0.5f),
                                        backButtonPos.y));
    
    levelChoices->SetIsExtended(true, false);

    //Calculate the best scale for the level list, keeping it at the right aspect ratio.
    Vector2f levelChoicesSize = levelChoices->GetBounds().GetDimensions();
    float aspectRatio = levelChoicesSize.x / levelChoicesSize.y;
    Vector2f center = halfWindow.FlipY(),
             size = (aspectRatio < 1.0f ?
                        Vector2f(fullWindow.y * aspectRatio, fullWindow.y) :
                        Vector2f(fullWindow.x, fullWindow.x / aspectRatio));
    levelChoices->SetBounds(Box2D(center, size));
}
#include "MainMenu.h"

#include "../../Rendering/GUI/GUI Elements/GUIPanel.h"
#include "../../Rendering/GUI/GUI Elements/GUITexture.h"

#include "PageManager.h"
#include "../Content/MenuContent.h"
#include "ChooseLevelEditor.h"


#define CREATE_BUTTON(ButtonVarName) \
            MenuContent::Instance.CreateGUITexture(&MenuContent::Instance.ButtonVarName, true)
#define CREATE_TEX(ButtonVarName) \
            MenuContent::Instance.CreateGUITexture(&MenuContent::Instance.ButtonVarName, false)


MainMenu::MainMenu(PageManager* manager)
    : Page(manager)
{
    playButton = GUIElementPtr(new GUITexture(CREATE_BUTTON(PlayButton)));
    ((GUITexture*)playButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*) pData;
    };
    ((GUITexture*)playButton.get())->OnClicked_pData = this;

    optionsButton = GUIElementPtr(new GUITexture(CREATE_BUTTON(OptionsButton)));
    ((GUITexture*)optionsButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*)pData;
    };
    ((GUITexture*)optionsButton.get())->OnClicked_pData = this;

    editorButton = GUIElementPtr(new GUITexture(CREATE_BUTTON(EditorButton)));
    ((GUITexture*)editorButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*)pData;
        std::string err;
        Page::Ptr newPage(new ChooseLevelEditor(mm->Manager, err));
        mm->Manager->CurrentPage = Page::Ptr(new ChooseLevelEditor(mm->Manager, err));
    };
    ((GUITexture*)editorButton.get())->OnClicked_pData = this;

    quitButton = GUIElementPtr(new GUITexture(CREATE_BUTTON(QuitButton)));
    ((GUITexture*)quitButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*)pData;
        mm->Manager->EndWorld();
    };
    ((GUITexture*)quitButton.get())->OnClicked_pData = this;

    background = GUIElementPtr(new GUITexture(CREATE_TEX(PageBackground)));

    RePositionGUI();

    GUIPanel* panel = new GUIPanel();
    panel->SetPosition(Vector2f());
    panel->AddElement(background);
    panel->AddElement(playButton);
    panel->AddElement(optionsButton);
    panel->AddElement(editorButton);
    panel->AddElement(quitButton);
    GUIManager.RootElement = GUIElementPtr(panel);
}

void MainMenu::OnWindowResized(void)
{
    RePositionGUI();
}
void MainMenu::OnCloseWindow(void)
{
    Manager->EndWorld();
}

void MainMenu::RePositionGUI(void)
{
    float halfX = 0.5f * (float)Manager->GetWindowSize().x,
          height = (float)Manager->GetWindowSize().y;

    MTexture2D* backTex = ((GUITexture*)background.get())->GetTex();
    Vector2f backTexSize((float)backTex->GetWidth(), (float)backTex->GetHeight());
    background->SetPosition(Vector2f(halfX, -0.5f * height));
    background->SetScale(Vector2f(halfX / (backTexSize.x * 0.5f),
                                  (height * 0.5f) / (backTexSize.y * 0.5f)));
    background->Depth = -0.01f;

    playButton->SetPosition(Vector2f(halfX, height * -0.635f));
    optionsButton->SetPosition(Vector2f(halfX, height * -0.45f));
    editorButton->SetPosition(Vector2f(halfX, height * -0.275f));
    quitButton->SetPosition(Vector2f(halfX, height * -0.105f));

    const Vector2f scale(0.75f, 0.75f);
    playButton->SetScale(scale);
    optionsButton->SetScale(scale);
    editorButton->SetScale(scale);
    quitButton->SetScale(scale);
}
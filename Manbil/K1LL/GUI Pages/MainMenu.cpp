#include "MainMenu.h"

#include "../../Rendering/GUI/GUI Elements/GUIPanel.h"
#include "../../Rendering/GUI/GUI Elements/GUITexture.h"

#include "PageManager.h"
#include "../Content/MenuContent.h"


#define MENU_CONTENT MenuContent::Instance


MainMenu::MainMenu(PageManager* manager)
    : Page(manager)
{
    playButton = GUIElementPtr(new GUITexture(MENU_CONTENT.CreateGUITexture(&MenuContent::Instance.PlayButton,
                                                                            true)));
    ((GUITexture*)playButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*) pData;
    };
    ((GUITexture*)playButton.get())->OnClicked_pData = this;

    optionsButton = GUIElementPtr(new GUITexture(MENU_CONTENT.CreateGUITexture(&MenuContent::Instance.OptionsButton,
                                                                               true)));
    ((GUITexture*)optionsButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*)pData;
    };
    ((GUITexture*)optionsButton.get())->OnClicked_pData = this;

    editorButton = GUIElementPtr(new GUITexture(MENU_CONTENT.CreateGUITexture(&MenuContent::Instance.EditorButton,
                                                                              true)));
    ((GUITexture*)editorButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*)pData;
    };
    ((GUITexture*)editorButton.get())->OnClicked_pData = this;

    quitButton = GUIElementPtr(new GUITexture(MENU_CONTENT.CreateGUITexture(&MenuContent::Instance.QuitButton,
                                                                            true)));
    ((GUITexture*)quitButton.get())->OnClicked = [](GUITexture* b, Vector2f mPos, void* pData)
    {
        MainMenu* mm = (MainMenu*)pData;
        mm->Manager->EndWorld();
    };
    ((GUITexture*)quitButton.get())->OnClicked_pData = this;

    background = GUIElementPtr(new GUITexture(MENU_CONTENT.CreateGUITexture(&MenuContent::Instance.Background, false)));

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

#include "../../DebugAssist.h"
void MainMenu::RePositionGUI(void)
{
    float halfX = 0.5f * (float)Manager->GetWindowSize().x,
          height = (float)Manager->GetWindowSize().y;

    background->SetPosition(Vector2f(halfX, -0.5f * height));
    background->SetScale(Vector2f(1.0f, (2.0f * halfX) / height));

    std::cout << "Pos: " << DebugAssist::ToString(background->GetPos()) << "; Background: " << DebugAssist::ToString(background->GetScale()) << "\n";

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
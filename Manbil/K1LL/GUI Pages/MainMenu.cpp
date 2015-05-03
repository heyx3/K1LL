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

void MainMenu::RePositionGUI(void)
{
    float halfX = 0.5f * (float)Manager->GetWindowSize().x,
          height = (float)Manager->GetWindowSize().y;

    background->SetPosition(Vector2f(halfX, 0.5f * height));

    playButton->SetPosition(Vector2f(halfX, height * -0.7f));
    optionsButton->SetPosition(Vector2f(halfX, height * -0.475f));
    editorButton->SetPosition(Vector2f(halfX, height * -0.25f));
    quitButton->SetPosition(Vector2f(halfX, -0.125f));
}
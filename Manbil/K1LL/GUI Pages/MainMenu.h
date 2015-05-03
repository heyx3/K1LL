#pragma once

#include "Page.h"


class MainMenu : public Page
{
public:

    MainMenu(PageManager* manager);

    virtual void OnWindowResized(void) override;
    virtual void OnCloseWindow(void) override;

private:

    GUIElementPtr playButton, optionsButton, editorButton, quitButton, background;


    void RePositionGUI(void);
};
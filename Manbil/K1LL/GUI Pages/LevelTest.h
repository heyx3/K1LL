#pragma once

#include "Page.h"

#include "../Game/Level/Level.h"


class LevelEditor;


//Lets the player test a level he's editing.
class LevelTest : public Page
{
public:

    Level Lvl;


    LevelTest(Page::Ptr editor, PageManager* manager,
              Vector2f playerStartPos, std::string& outErrorMsg);


    virtual void Update(Vector2i mousePos, float frameSeconds) override;
    virtual void Render(float frameSeconds) override;
    
    virtual void OnWindowResized(void) override;
    virtual void OnCloseWindow(void) override;

    virtual void OnOtherWindowEvent(sf::Event& windowEvent) override;

    
private:

    //The amount the mouse wheel has scrolled since last frame.
    int scrollWheel = 0;

    Page::Ptr levelEditor;
};
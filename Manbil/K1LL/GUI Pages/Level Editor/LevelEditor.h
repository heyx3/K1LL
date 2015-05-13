#pragma once

#include "../Page.h"


class LevelEditor : public Page
{
public:

    //The different mouse buttons that can be clicked.
    enum MouseClickType
    {
        MCT_LEFT,
        MCT_RIGHT,
    };


    //If something went wrong, a message is output into the given string.
    LevelEditor(const std::string& levelFileName, PageManager* manager, std::string& outErrorMsg);
    ~LevelEditor(void);


    virtual void Update(Vector2i mousePos, float frameseconds) override;
    virtual void Render(float frameSeconds) override;

    virtual void OnWindowResized(void) override;
    virtual void OnCloseWindow(void) override;

    virtual void OnOtherWindowEvent(sf::Event& windowEvent) override;
    

private:

    Box2D worldViewBounds;
};
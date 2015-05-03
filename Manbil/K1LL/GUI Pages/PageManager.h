#pragma once

#include "../../Game Loop/SFMLOpenGLWorld.h"

#include "Page.h"


//Handles the different pages in K1LL.
class PageManager : public SFMLOpenGLWorld
{
public:

    Page::Ptr CurrentPage;


    PageManager(void);


    Vector2u GetWindowSize(void) const { return windowSize; }


protected:

    virtual sf::VideoMode GetModeToUse(unsigned int windowW, unsigned int windowH) override;
    virtual std::string GetWindowTitle(void) override;
    virtual sf::Uint32 GetSFStyleFlags(void) override;

    virtual void InitializeWorld(void) override;
    virtual void OnWorldEnd(void) override;

    virtual void UpdateWorld(float elapsedSeconds) override;
    virtual void RenderOpenGL(float elapsedSeconds) override;

    virtual void OnInitializeError(std::string errorMsg) override;

    virtual void OnWindowResized(unsigned int newWidth, unsigned int newHeight) override;
    virtual void OnCloseWindow(void) override;
    virtual void OnWindowLostFocus(void) override;
    virtual void OnWindowGainedFocus(void) override;
    virtual void OnOtherWindowEvent(sf::Event& windowEvent) override;


private:

    Vector2u windowSize;
};
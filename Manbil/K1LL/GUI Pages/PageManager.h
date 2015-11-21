#pragma once

#include "../../Game Loop/SFMLOpenGLWorld.h"

#include "Page.h"


//Handles the different pages in K1LL.
class PageManager : public SFMLOpenGLWorld
{
public:


    PageManager(void);


    Vector2u GetWindowSize(void) const { return windowSize; }
    
    const Page::Ptr GetCurrentPage(void) const { return currentPage; }

    //Sets the given page to become the new page next update step.
    void UpdateCurrentPage(Page::Ptr newPage) { nextPage = newPage; }


protected:

    virtual sf::VideoMode GetModeToUse(unsigned int windowW, unsigned int windowH) override;
    virtual std::string GetWindowTitle(void) override;
    virtual sf::Uint32 GetSFStyleFlags(void) override;
    virtual sf::ContextSettings GenerateContext(void) override;

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
    
    Page::Ptr currentPage, nextPage;

    Vector2u windowSize;
};
#include "PageManager.h"

#include <iostream>

#include "../../Rendering/GUI/GUIMaterials.h"
#include "../../Rendering/Data Nodes/DataNodes.hpp"
#include "../../Rendering/GUI/TextRenderer.h"

#include "../Content/ContentLoader.h"
#include "MainMenu.h"



PageManager::PageManager(void)
    : windowSize(800, 900),
      SFMLOpenGLWorld(800, 900, sf::ContextSettings())
{

}

sf::VideoMode PageManager::GetModeToUse(unsigned int windowW, unsigned int windowH)
{
    //Change this return value to change the window resolution mode.
    //To use native fullscreen, return "sf::VideoMode::getFullscreenModes()[0];".
    return sf::VideoMode(windowW, windowH);
}
std::string PageManager::GetWindowTitle(void)
{
    return "K1LL";
}
sf::Uint32 PageManager::GetSFStyleFlags(void)
{
    return sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;
}


void PageManager::InitializeWorld(void)
{
    SFMLOpenGLWorld::InitializeWorld();
    if (IsGameOver())
    {
        return;
    }

    std::string err;

    ContentLoader::LoadContent(err);
    if (!err.empty())
    {
        std::cout << err << "\n";
        char dummy;
        std::cin >> dummy;

        EndWorld();
        return;
    }


    currentPage = Page::Ptr(new MainMenu(this));
}
void PageManager::OnWorldEnd(void)
{
    currentPage.reset();
    ContentLoader::DestroyContent();
}

void PageManager::UpdateWorld(float frameSeconds)
{
    //Calculate mouse pos.
    sf::Vector2i screenPos = sf::Mouse::getPosition(*GetWindow());

    if (nextPage.get() != 0)
    {
        currentPage = nextPage;
        nextPage = 0;
    }
    currentPage->Update(Vector2i(screenPos.x, -screenPos.y), frameSeconds);
}
void PageManager::RenderOpenGL(float frameSeconds)
{
    glViewport(0, 0, windowSize.x, windowSize.y);

    currentPage->Render(frameSeconds);
}

void PageManager::OnInitializeError(std::string errorMsg)
{
    SFMLOpenGLWorld::OnInitializeError(errorMsg);
    EndWorld();
}

void PageManager::OnWindowResized(unsigned int newWidth, unsigned int newHeight)
{
    windowSize.x = newWidth;
    windowSize.y = newHeight;

    currentPage->OnWindowResized();
}
void PageManager::OnCloseWindow(void)
{
    currentPage->OnCloseWindow();
}
void PageManager::OnWindowLostFocus(void)
{
    currentPage->OnWindowLostFocus();
}
void PageManager::OnWindowGainedFocus(void)
{
    currentPage->OnWindowGainedFocus();
}
void PageManager::OnOtherWindowEvent(sf::Event& windowEvent)
{
    currentPage->OnOtherWindowEvent(windowEvent);
}
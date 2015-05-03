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

    DrawingQuad::InitializeQuad();

    std::string err = TextRenderer::InitializeSystem();
    if (!err.empty())
    {
        std::cout << "Error initializing text renderer: " << err << "\n";
        char dummy;
        std::cin >> dummy;

        EndWorld();
        return;
    }

    ContentLoader::LoadContent(err);
    if (!err.empty())
    {
        std::cout << err << "\n";
        char dummy;
        std::cin >> dummy;

        EndWorld();
        return;
    }


    CurrentPage = Page::Ptr(new MainMenu(this));
}
void PageManager::OnWorldEnd(void)
{
    CurrentPage.reset();
    TextRenderer::DestroySystem();
    DrawingQuad::DestroyQuad();
}

void PageManager::UpdateWorld(float frameSeconds)
{
    //Calculate mouse pos.
    sf::Vector2i screenPos = sf::Mouse::getPosition();
    sf::Vector2i mPosFinal = screenPos - GetWindow()->getPosition() - sf::Vector2i(5, 30);
    mPosFinal.y -= windowSize.y;

    CurrentPage->Update(Vector2i(mPosFinal.x, mPosFinal.y), frameSeconds);
}
void PageManager::RenderOpenGL(float frameSeconds)
{
    CurrentPage->Render(frameSeconds);
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

    CurrentPage->OnWindowResized();
}
void PageManager::OnCloseWindow(void)
{
    CurrentPage->OnCloseWindow();
}
void PageManager::OnWindowLostFocus(void)
{
    CurrentPage->OnWindowLostFocus();
}
void PageManager::OnWindowGainedFocus(void)
{
    CurrentPage->OnWindowGainedFocus();
}
void PageManager::OnOtherWindowEvent(sf::Event& windowEvent)
{
    CurrentPage->OnOtherWindowEvent(windowEvent);
}
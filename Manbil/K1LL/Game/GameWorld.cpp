#include "GameWorld.h"

#include "../../DebugAssist.h"

#include "../Actors/HumanPlayer.h"
#include "../Content/ActorContent.h"


GameWorld::GameWorld(void)
    : windowSize(800, 600),
      SFMLOpenGLWorld(800, 600, sf::ContextSettings(16, 0, 0, 4, 1))
{

}


void GameWorld::InitializeWorld(void)
{
    SFMLOpenGLWorld::InitializeWorld();
    if (IsGameOver())
    {
        return;
    }

    assert(wind == 0);
    wind = ((SFMLOpenGLWorld*)this)->GetWindow();

    std::string err;
    ActorContent::Instance.Initialize(err);
    if (!err.empty())
    {
        std::cout << "Error initializing actor content: " << err << "\n";
        char dummy;
        std::cin >> dummy;
        EndWorld();
        return;
    }
}
void GameWorld::OnWorldEnd(void)
{
    wind = 0;
    ActorContent::Instance.Destroy();
}

void GameWorld::UpdateWorld(float elapsedSeconds)
{
    //Calculate mouse pos.
    sf::Vector2i screenPos = sf::Mouse::getPosition();
    sf::Vector2i mPosFinal = screenPos - GetWindow()->getPosition() - sf::Vector2i(5, 30);
    mPosFinal.y -= windowSize.y;

    mPos = Vector2i(mPosFinal.x, mPosFinal.y);
}
void GameWorld::RenderOpenGL(float elapsedSeconds)
{
    glViewport(0, 0, windowSize.x, windowSize.y);
    ScreenClearer().ClearScreen();
    RenderingState(RenderingState::C_NONE).EnableState();

    
}

void GameWorld::OnWindowResized(unsigned int newWidth, unsigned int newHeight)
{
    windowSize.x = newWidth;
    windowSize.y = newHeight;
}
void GameWorld::OnInitializeError(std::string errorMsg)
{
    SFMLOpenGLWorld::OnInitializeError(errorMsg);
    EndWorld();
}
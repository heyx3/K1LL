#include "LevelTest.h"

#include "Level Editor/LevelEditor.h"
#include "../Game/InputHandler.h"
#include "../Game/Players/HumanPlayer.h"
#include "../Game/Rendering/PlayerViewport.h"
#include "PageManager.h"

#include "../Game/Players/Weapons/Puncher.h"


namespace
{
    MatchInfo MakeMatchInfo(void)
    {
        return MatchInfo(Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), 0, 1,
                         [](Level& lvl) { return Weapon::Ptr(new Puncher(lvl)); },
                         [](Level& lvl) { return Weapon::Ptr(new Puncher(lvl)); },
                         [](Level& lvl) { return Weapon::Ptr(new Puncher(lvl)); });
        //TODO: Add a heavy and special weapon here instead of three Punchers.
    }
}

LevelTest::LevelTest(Page::Ptr editor, PageManager* manager,
                     Vector2f playerPos, std::string& err)
    : levelEditor(editor), Lvl(((LevelEditor*)editor.get())->LevelData, MakeMatchInfo(), err),
      Page(manager)
{
    if (!Assert(err.empty(), "Error initializing fields", err))
    {
        return;
    }

    
    //Add a player to the level and create his viewport.

    Weapon::Ptr weaps[3] = {
        Lvl.MatchData.LightWeapon(Lvl),
        Lvl.MatchData.HeavyWeapon(Lvl),
        Lvl.MatchData.SpecialWeapon(Lvl)
    };
    Lvl.Players.push_back(PlayerPtr(new HumanPlayer(&Lvl, IM_KEYBOARD_MOUSE, playerPos, weaps)));

    PlayerViewport* playerView = new PlayerViewport(Lvl, Lvl.Players[0].get(), Manager, err);

    if (!Assert(err.empty(), "Error initializing player view", err))
    {
        delete playerView;
        return;
    }

    GUIManager.SetRoot(GUIElementPtr(playerView));
    GUIManager.GetRoot()->SetBounds(Box2D(0.0f, (float)Manager->GetWindow()->getSize().x,
                                          -(float)Manager->GetWindow()->getSize().y, 0.0f));
}


void LevelTest::Update(Vector2i mousePos, float frameSeconds)
{
    if (GetIsWindowInFocus())
    {
        InputHandler::Update(Manager->GetWindow(), scrollWheel);
        scrollWheel = 0;
    }

    Lvl.Update(frameSeconds);

    if (InputHandler::Values[IM_KEYBOARD_MOUSE].Pause)
    {
        Manager->UpdateCurrentPage(levelEditor);
    }

    Page::Update(mousePos, frameSeconds);
}
void LevelTest::Render(float frameSeconds)
{
    Page::Render(frameSeconds);
}
    
void LevelTest::OnWindowResized(void)
{
    GUIManager.GetRoot()->SetBounds(Box2D(0.0f, (float)Manager->GetWindow()->getSize().x,
                                          -(float)Manager->GetWindow()->getSize().y, 0.0f));
}
void LevelTest::OnCloseWindow(void)
{
    Manager->EndWorld();
}

void LevelTest::OnOtherWindowEvent(sf::Event& windowEvent)
{
    if (windowEvent.type == sf::Event::EventType::MouseWheelMoved)
    {
        scrollWheel = windowEvent.mouseWheel.delta;
    }
}
#pragma once

#include <string>
#include <memory>

#include <SFML/Window/Event.hpp>

#include "../../Math/Lower Math/Vectors.h"
#include "../../Rendering/GUI/GUIManager.h"


class PageManager;

//A game screen (e.x. "Main Menu", "Game", "Level Editor").
class Page
{
public:

    typedef std::shared_ptr<Page> Ptr;

    PageManager* Manager;
    GUIManager GUIManager;


    Page(PageManager* manager) : Manager(manager) { }


    virtual void Update(Vector2i mousePos, float frameSeconds);
    virtual void Render(float frameSeconds);

    virtual void OnWindowResized(void) = 0;
    virtual void OnCloseWindow(void) = 0;

    virtual void OnOtherWindowEvent(sf::Event& windowEvent) { }


    bool GetIsWindowInFocus(void) const { return isWindowInFocus; }

    virtual void OnWindowLostFocus(void) { isWindowInFocus = false; }
    virtual void OnWindowGainedFocus(void) { isWindowInFocus = true; }


protected:

    //If the given test is false, prints the given error, pauses the program, and ends the game world.
    //Returns whether the given test value is true.
    bool Assert(bool test, const std::string& errorIntro, const std::string& error);


private:

    bool isWindowInFocus = true;

    bool waitForMouseRelease = true;
};
#pragma once

#include "../../Game Loop/SFMLOpenGLWorld.h"


class GameWorld : public SFMLOpenGLWorld
{
public:

    
    //Gets the pixel coordinates of the mouse relative to the top-left of the window.
    static Vector2i GetMousePos(void) { return mPos; }

    static sf::RenderWindow* GetWindow(void) { return wind; }


    GameWorld(void);


protected:

    virtual sf::VideoMode GetModeToUse(unsigned int windowW, unsigned int windowH) override
    {
        return sf::VideoMode(windowW, windowH, 32);
    }
    virtual std::string GetWindowTitle(void) override
    {
        return "World window";
    }
    virtual sf::Uint32 GetSFStyleFlags(void) override
    {
        return sf::Style::Default;
    }

    virtual void InitializeWorld(void) override;
    virtual void OnWorldEnd(void) override;

    virtual void UpdateWorld(float elapsedSeconds) override;
    virtual void RenderOpenGL(float elapsedSeconds) override;

	virtual void OnWindowResized(unsigned int newWidth, unsigned int newHeight) override;
    virtual void OnInitializeError(std::string errorMsg) override;


private:

    static Vector2i mPos;
    static sf::RenderWindow* wind;


    Vector2u windowSize;
};
#include "ContentLoader.h"

#include <iostream>

#include "Constants.h"
#include "Settings.h"
#include "ActorContent.h"
#include "MenuContent.h"


void ContentLoader::LoadContent(std::string& err)
{
    //Initialize engine systems.
    DrawingQuad::InitializeQuad();
    err = TextRenderer::InitializeSystem();
    if (!err.empty())
    {
        err = "Error initializing text rendering system: " + err;
        return;
    }

    //Constants.
    Constants::Instance.ReadFromFile(err);
    if (!err.empty())
    {
        //The file doesn't exist, so create it.
        Constants::Instance.SaveToFile(err);
        if (!err.empty())
        {
            err = "Error saving constants data to file: " + err;
            return;
        }
    }

    //Settings.
    Settings::Instance.ReadFromFile();
    if (!err.empty())
    {
        return;
    }

    //ActorContent.
    if (!ActorContent::Instance.Initialize(err))
    {
        err = "Error loading actor content: " + err;
        return;
    }

    //MenuContent.
    if (!MenuContent::Instance.Initialize(err))
    {
        err = "Error loading menu content: " + err;
        return;
    }
}
void ContentLoader::DestroyContent(void)
{
    ActorContent::Instance.Destroy();
    MenuContent::Instance.Destroy();

    TextRenderer::DestroySystem();
    DrawingQuad::DestroyQuad();

    std::string err;
    Settings::Instance.SaveToFile(err);
    if (!err.empty())
    {
        std::cout << "Error saving settings file: " << err;
        char pause;
        std::cin >> pause;
    }
}
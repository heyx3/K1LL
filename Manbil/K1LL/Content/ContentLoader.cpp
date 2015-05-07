#include "ContentLoader.h"

#include "Constants.h"
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
    std::string tryRead;
    Constants::Instance.ReadFromFile(tryRead);
    if (!tryRead.empty())
    {
        Constants::Instance.SaveToFile(err);
        if (!err.empty())
        {
            err = "Error saving constants data to file: " + err;
            return;
        }
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
}
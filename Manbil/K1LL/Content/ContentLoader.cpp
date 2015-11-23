#include "ContentLoader.h"

#include <iostream>

#include "LevelConstants.h"
#include "WeaponConstants.h"
#include "Settings.h"
#include "QualitySettings.h"
#include "MenuContent.h"
#include "ActorContent.h"
#include "WeaponContent.h"
#include "BulletContent.h"
#include "ParticleContent.h"


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
    LevelConstants::Instance.ReadFromFile(err);
    if (!err.empty())
    {
        //The file doesn't exist, so create it.
        LevelConstants::Instance.SaveToFile(err);
        if (!err.empty())
        {
            err = "Error saving level constants data to file: " + err;
            return;
        }
    }
    WeaponConstants::Instance.ReadFromFile(err);
    if (!err.empty())
    {
        //The file doesn't exist, so create it.
        WeaponConstants::Instance.SaveToFile(err);
        if (!err.empty())
        {
            err = "Error saving weapon constants data to file: " + err;
            return;
        }
    }

    //Settings.
    Settings::Instance.Initialize();
    QualitySettings::Instance.Initialize();

    //Content.
    if (!MenuContent::Instance.Initialize(err))
    {
        err = "Error loading menu content: " + err;
        return;
    }
    if (!ActorContent::Instance.Initialize(err))
    {
        err = "Error loading actor content: " + err;
        return;
    }
    if (!WeaponContent::Instance.Initialize(err))
    {
        err = "Error loading weapon content: " + err;
        return;
    }
    if (!BulletContent::Instance.Initialize(err))
    {
        err = "Error loading bullet content: " + err;
        return;
    }
    if (!ParticleContent::Instance.Initialize(err))
    {
        err = "Error loading particle content: " + err;
        return;
    }
}
void ContentLoader::DestroyContent(void)
{
    MenuContent::Instance.Destroy();
    ActorContent::Instance.Destroy();
    WeaponContent::Instance.Destroy();
    BulletContent::Instance.Destroy();
    ParticleContent::Instance.Destroy();

    TextRenderer::DestroySystem();
    DrawingQuad::DestroyQuad();


    //Save the settings to a file so they can be loaded next time the game starts up.
    std::string err;
    Settings::Instance.SaveToFile(err);
    if (!err.empty())
    {
        std::cout << "Error saving settings file: " << err;
        char pause;
        std::cin >> pause;
    }
    QualitySettings::Instance.SaveToFile(err);
    if (!err.empty())
    {
        std::cout << "Error saving quality settings file: " << err;
        char pause;
        std::cin >> pause;
    }
}
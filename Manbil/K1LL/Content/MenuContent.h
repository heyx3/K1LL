#pragma once

#include "../../Rendering/GUI/GUI Elements/GUITexture.h"


//Contains content for menus.
class MenuContent
{
public:

    static MenuContent Instance;


    MTexture2D Background, BackTex;

    //Main Menu textures.
    MTexture2D PlayButton, OptionsButton, EditorButton, QuitButton;

    //Level Editor textures.
    MTexture2D ConfirmDeletePopup, NOTex, YESTex,
               EditLevelTex, DeleteLevelTex,
               CreateLevelTex,
               LevelSelectionBoxHighlight, LevelSelectionBoxBackground;


    bool Initialize(std::string& outErrorMsg);
    void Destroy(void);


    //Creates a GUI texture for a menu.
    //The texture must be color!
    GUITexture CreateGUITexture(MTexture2D* tex, bool isButton);


private:

    MenuContent(void);


    Material *staticColorGUIMat, *animatedColorGUIMat;
    UniformDictionary staticColorGUIParams, animatedColorGUIParams;
};
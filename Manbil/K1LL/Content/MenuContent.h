#pragma once

#include "../../Rendering/GUI/GUI Elements/GUITexture.h"
#include "../../Rendering/GUI/GUI Elements/GUILabel.h"

#include "../Level Info/ItemTypes.h"


//Contains content for menus.
class MenuContent
{
public:

    static MenuContent Instance;


    //Materials.
    Material *StaticColorGUIMat = 0,
             *StaticColorGUIMatNoTex = 0,
             *AnimatedColorGUIMat = 0,
             *LabelGUIMat = 0;
    UniformDictionary StaticColorGUIParams, StaticColorGUINoTexParams,
                      AnimatedColorGUIParams,
                      LabelGUIParams;

    //General-purpose textures.
    MTexture2D PageBackground, TextBoxBackground, BackButton;

    //Main Menu textures.
    MTexture2D PlayButton, OptionsButton, EditorButton, QuitButton;

    //"Choose level to edit" textures.
    MTexture2D ConfirmDeletePopup, NOTex, YESTex,
               EditLevelTex, DeleteLevelTex,
               CreateLevelTex;

    //Level selection textures.
    MTexture2D LevelSelectionBoxHighlight, LevelSelectionBoxBackground, LevelSelectionSingleElement;

    //2D Room-rendering textures.
    MTexture2D FloorTex, WallTex,
               AmmoLightTex, AmmoHeavyTex, AmmoSpecialTex,
               HealthTex;

    //Level editor textures.
    MTexture2D EditorNoiseTex;


    //Text-rendering stuff.

    TextRenderer TextRender;

    FreeTypeHandler::FontID MainTextFont;
    unsigned int MainTextFontHeight;
    Vector2f MainTextFontScale;


    bool Initialize(std::string& outErrorMsg);
    void Destroy(void);


    MTexture2D* GetPickupTex(ItemTypes pickup);


    //Creates a GUI texture for a menu. A 0 pointer may be passed for the texture.
    //The texture must be color, not greyscale!
    GUITexture CreateGUITexture(MTexture2D* tex, bool isButton);

    //Creates a GUI label for a menu.
    //If the default value is passed for the slot, a new slot is generated.
    //If an error happens when generating a new slot, the error message is output into the given string.
    GUILabel CreateGUILabel(FreeTypeHandler::FontID font, unsigned int fontHeight, Vector2f fontScale,
                            unsigned int renderWidth, std::string& outErrorMsg,
                            GUILabel::HorizontalOffsets horz, GUILabel::VerticalOffsets vert,
                            unsigned int slot = FreeTypeHandler::ERROR_ID,
                            Vector3f color = Vector3f(1.0f, 1.0f, 1.0f));


private:

    MenuContent(void);
};
#include "MenuContent.h"

#include "../../Rendering/GUI/GUIMaterials.h"
#include "../../Rendering/Data Nodes/DataNodes.hpp"


MenuContent MenuContent::Instance = MenuContent();

MenuContent::MenuContent(void)
    : Background(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      BackTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      PlayButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      OptionsButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      EditorButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      QuitButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      ConfirmDeletePopup(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      NOTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      YESTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      EditLevelTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      DeleteLevelTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      CreateLevelTex(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      LevelSelectionBoxHighlight(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      LevelSelectionBoxBackground(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false)
{

}

bool MenuContent::Initialize(std::string& err)
{
    Background.Create();
    BackTex.Create();

    PlayButton.Create();
    OptionsButton.Create();
    EditorButton.Create();
    QuitButton.Create();

    ConfirmDeletePopup.Create();
    NOTex.Create();
    YESTex.Create();
    EditLevelTex.Create();
    DeleteLevelTex.Create();
    CreateLevelTex.Create();
    LevelSelectionBoxHighlight.Create();
    LevelSelectionBoxBackground.Create();


#define TRY_LOAD(buttonVar, fileName) \
    buttonVar.SetDataFromFile(std::string("Content/Menu/") + #fileName, err); \
    if (!err.empty()) \
    { \
        err = std::string("Error loading ") + #fileName + " tex: " + err; \
        return false; \
    }
    
    TRY_LOAD(Background, Background.png)
    TRY_LOAD(BackTex, BackButton.png)

    TRY_LOAD(PlayButton, Play Button.png)
    TRY_LOAD(OptionsButton, Options Button.png)
    TRY_LOAD(EditorButton, Editor Button.png)
    TRY_LOAD(QuitButton, Quit Button.png)

    TRY_LOAD(ConfirmDeletePopup, ConfirmDeletePopup.png)
    TRY_LOAD(NOTex, NOWithoutBackground.png)
    TRY_LOAD(YESTex, YESWithoutBackground.png)
    TRY_LOAD(EditLevelTex, EditLevelButton.png)
    TRY_LOAD(DeleteLevelTex, DeleteLevelButton.png)
    TRY_LOAD(CreateLevelTex, CreateButton.png)
    TRY_LOAD(LevelSelectionBoxHighlight, LevelChoiceHighlight.png)
    TRY_LOAD(LevelSelectionBoxBackground, LevelChoiceBackground.png)

    
    auto genM = GUIMaterials::GenerateStaticQuadDrawMaterial(staticColorGUIParams,
                                                             GUIMaterials::TextureTypes::TT_COLOR);
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating static color GUI mat: " + genM.ErrorMessage;
        return false;
    }
    staticColorGUIMat = genM.Mat;


    typedef DataNode::Ptr DNP;
    DNP lerpParam(new ParamNode(1, GUIMaterials::DynamicQuadDraw_TimeLerp, "timeLerpParam"));
    DNP lerpColor(new InterpolateNode(Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
                                      Vector4f(0.5f, 0.5f, 0.5f, 1.0f),
                                      lerpParam, InterpolateNode::IT_Linear, "lerpColor")),
        lerpSize(new InterpolateNode(Vector2f(1.0f, 1.0f), Vector2f(1.1f, 1.1f),
                                     lerpParam, InterpolateNode::IT_Linear, "lerpSize"));
    genM = GUIMaterials::GenerateDynamicQuadDrawMaterial(animatedColorGUIParams,
                                                         GUIMaterials::TextureTypes::TT_COLOR,
                                                         lerpSize, lerpColor);
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating static color GUI mat: " + genM.ErrorMessage;
        return false;
    }
    animatedColorGUIMat = genM.Mat;


    return true;
}
void MenuContent::Destroy(void)
{
    Background.DeleteIfValid();
    BackTex.DeleteIfValid();

    PlayButton.DeleteIfValid();
    OptionsButton.DeleteIfValid();
    EditorButton.DeleteIfValid();
    QuitButton.DeleteIfValid();
    
    ConfirmDeletePopup.DeleteIfValid();
    NOTex.DeleteIfValid();
    YESTex.DeleteIfValid();
    EditLevelTex.DeleteIfValid();
    DeleteLevelTex.DeleteIfValid();
    CreateLevelTex.DeleteIfValid();
    LevelSelectionBoxHighlight.DeleteIfValid();
    LevelSelectionBoxBackground.DeleteIfValid();
}

GUITexture MenuContent::CreateGUITexture(MTexture2D* tex, bool isButton)
{
    assert(tex->IsColorTexture());
    if (isButton)
    {
        return GUITexture(animatedColorGUIParams, tex, animatedColorGUIMat, true, 7.5f);
    }
    else
    {
        return GUITexture(staticColorGUIParams, tex, staticColorGUIMat, false);
    }
}
#include "MenuContent.h"

#include "../../Rendering/GUI/GUIMaterials.h"
#include "../../Rendering/Data Nodes/DataNodes.hpp"


MenuContent MenuContent::Instance = MenuContent();

MenuContent::MenuContent(void)
    : PageBackground(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      BackButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      TextBoxBackground(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
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
      LevelSelectionBoxBackground(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      MainTextFontScale(0.25f, 0.25f), MainTextFontHeight(256)
{

}

bool MenuContent::Initialize(std::string& err)
{
    #pragma region Load textures

    PageBackground.Create();
    BackButton.Create();
    TextBoxBackground.Create();

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
    
    TRY_LOAD(PageBackground, Background.png)
    TRY_LOAD(BackButton, BackButton.png)
    TRY_LOAD(TextBoxBackground, TextBoxBackground.png)

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

    #pragma endregion

    #pragma region Load fonts

    MainTextFont = TextRender.CreateAFont("Content/Menu/BodyFont.ttf", err, 25);
    MainTextFontScale = Vector2f(1.0f, 1.0f) * 0.25f;

    if (!err.empty())
    {
        err = "Error loading 'Menu/BodyFont.ttf': " + err;
        return false;
    }

    #pragma endregion
    
    #pragma region Create materials

    auto genM = GUIMaterials::GenerateStaticQuadDrawMaterial(StaticColorGUIParams,
                                                             GUIMaterials::TT_COLOR);
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating static color GUI mat: " + genM.ErrorMessage;
        return false;
    }
    StaticColorGUIMat = genM.Mat;

    genM = GUIMaterials::GenerateStaticQuadDrawMaterial(LabelGUIParams, GUIMaterials::TT_TEXT);
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating label GUI mat: " + genM.ErrorMessage;
        return false;
    }
    LabelGUIMat = genM.Mat;


    typedef DataNode::Ptr DNP;
    DNP lerpParam(new ParamNode(1, GUIMaterials::DynamicQuadDraw_TimeLerp, "timeLerpParam"));
    DNP lerpColor(new InterpolateNode(Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
                                      Vector4f(0.5f, 0.5f, 0.5f, 1.0f),
                                      lerpParam, InterpolateNode::IT_Linear, "lerpColor")),
        lerpSize(new InterpolateNode(Vector2f(1.0f, 1.0f), Vector2f(1.1f, 1.1f),
                                     lerpParam, InterpolateNode::IT_Linear, "lerpSize"));
    genM = GUIMaterials::GenerateDynamicQuadDrawMaterial(AnimatedColorGUIParams,
                                                         GUIMaterials::TextureTypes::TT_COLOR,
                                                         lerpSize, lerpColor);
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating static color GUI mat: " + genM.ErrorMessage;
        return false;
    }
    AnimatedColorGUIMat = genM.Mat;

    #pragma endregion


    return true;
}
void MenuContent::Destroy(void)
{
    bool b = TextRender.DeleteFont(MainTextFont);
    assert(b);


    delete StaticColorGUIMat, AnimatedColorGUIMat, LabelGUIMat;
    StaticColorGUIMat = 0;
    AnimatedColorGUIMat = 0;
    LabelGUIMat = 0;
    StaticColorGUIParams.ClearUniforms();
    AnimatedColorGUIParams.ClearUniforms();
    LabelGUIParams.ClearUniforms();


    PageBackground.DeleteIfValid();
    BackButton.DeleteIfValid();
    TextBoxBackground.DeleteIfValid();

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
    assert(tex == 0 || tex->IsColorTexture());
    if (isButton)
    {
        return GUITexture(AnimatedColorGUIParams, tex, AnimatedColorGUIMat, true, 7.5f);
    }
    else
    {
        return GUITexture(StaticColorGUIParams, tex, StaticColorGUIMat, false);
    }
}
GUILabel MenuContent::CreateGUILabel(FreeTypeHandler::FontID font, unsigned int height, Vector2f scale,
                                     unsigned int renderWidth, std::string& err,
                                     GUILabel::HorizontalOffsets horz, GUILabel::VerticalOffsets vert,
                                     unsigned int slot, Vector3f color)
{
    TextRenderer::FontSlot fs(font, slot);

    if (slot == FreeTypeHandler::ERROR_ID)
    {
        fs = TextRender.CreateTextRenderSlot(font, err, renderWidth, height, false,
                                             TextureSampleSettings2D(FT_LINEAR, WT_CLAMP));
        if (!err.empty())
        {
            err = "Error generating font slot: " + err;
            return GUILabel();
        }
    }

    GUILabel retVal(LabelGUIParams, &TextRender, fs,  LabelGUIMat, 1.0f, horz, vert);
    retVal.SetColor(Vector4f(color, 1.0f));
    return retVal;
}
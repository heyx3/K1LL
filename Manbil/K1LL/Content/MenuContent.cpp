#include "MenuContent.h"

#include "../../Rendering/GUI/GUIMaterials.h"
#include "../../Rendering/Data Nodes/DataNodes.hpp"


MenuContent MenuContent::Instance = MenuContent();

MenuContent::MenuContent(void)
    : PlayButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      OptionsButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      EditorButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      QuitButton(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      Background(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PixelSizes::PS_8U, false),
      ButtonScale(1.0f, 1.0f)
{

}

bool MenuContent::Initialize(std::string& err)
{
    PlayButton.Create();
    OptionsButton.Create();
    EditorButton.Create();
    QuitButton.Create();
    Background.Create();

    PlayButton.SetDataFromFile("Content/Menu/Play Button.png", err);
    if (!err.empty())
    {
        err = "Error loading Play Button tex: " + err;
        return false;
    }

    OptionsButton.SetDataFromFile("Content/Menu/Options Button.png", err);
    if (!err.empty())
    {
        err = "Error loading Options Button tex: " + err;
        return false;
    }

    EditorButton.SetDataFromFile("Content/Menu/Editor Button.png", err);
    if (!err.empty())
    {
        err = "Error loading Editor Button tex: " + err;
        return false;
    }

    QuitButton.SetDataFromFile("Content/Menu/Quit Button.png", err);
    if (!err.empty())
    {
        err = "Error loading Quit Button tex: " + err;
        return false;
    }

    Background.SetDataFromFile("Content/Menu/Background.png", err);
    if (!err.empty())
    {
        err = "Error loading Background tex: " + err;
        return false;
    }

    
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
    PlayButton.DeleteIfValid();
    OptionsButton.DeleteIfValid();
    EditorButton.DeleteIfValid();
    QuitButton.DeleteIfValid();
    Background.DeleteIfValid();
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
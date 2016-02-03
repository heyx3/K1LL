#include "PostProcessing.h"

#include "../../Rendering/Primitives/DrawingQuad.h"
#include "QualitySettings.h"


namespace
{

}


PostProcessing PostProcessing::Instance = PostProcessing();
std::string PostProcessing::errMsg = std::string();


PostProcessing::PostProcessing(void)
    : outTex(TextureSampleSettings2D(FT_NEAREST, WT_CLAMP), PS_8U, false)
{

}


bool PostProcessing::Initialize(std::string& outError)
{
    //See if there was an error on construction.
    outError = errMsg;
    if (!outError.empty())
        return false;


    //Set up the output texture and render target.
    outTex.Create();
    rendTarg.reset(new RenderTarget(PS_16U_DEPTH, outError));
    rendTarg->SetColorAttachment(RenderTargetTex(&outTex), false);


    RenderIOAttributes quadAttrs = DrawingQuad::GetVertexInputData();

    #pragma region Linearize HDR Pass
    {
        //HDR curve used: http://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/140

        std::string vShader = MaterialConstants::GetVertexHeader("out vec2 fIn_UV;\n", quadAttrs,
                                                                 MaterialUsageFlags()) +
R"(
void main()
{
    fIn_UV = )" + quadAttrs.GetAttribute(1).Name + R"( ;
    gl_Position = vec4( )" + quadAttrs.GetAttribute(0).Name + R"( , 1.0);
}
)";
        std::string fShader = MaterialConstants::GetFragmentHeader("in vec2 fIn_UV;\n",
                                                                   "out vec4 fOut_Color;\n",
                                                                   MaterialUsageFlags()) +
R"(
uniform sampler2D u_tex;

void main()
{
    vec4 inCol = texture2D(u_tex, fIn_UV);
    vec4 temp = max(vec4(0.0), inCol - 0.004);
    fOut_Color = (temp * ((6.2 * temp) + 0.5)) /
                 ((temp * ((6.2 * temp) + 1.7)) + 0.06);
    fOut_Color = inCol;
})";

        passParams.push_back(UniformDictionary());
        UniformDictionary& params = passParams[passParams.size() - 1];
        params["u_tex"] = Uniform("u_tex", UT_VALUE_SAMPLER2D);

        Material* mat = new Material(vShader, fShader, params, quadAttrs,
                                     BlendMode::GetOpaque(), outError);
        if (!outError.empty())
        {
            passParams.erase(passParams.end() - 1);
            return false;
        }

        passes.push_back(std::unique_ptr<Material>(mat));
    }
    #pragma endregion

    if (RenderTarget::GetCurrentTarget() != 0)
    {
        RenderTarget::GetCurrentTarget()->DisableDrawingInto();
    }

    return true;
}
void PostProcessing::Destroy(void)
{
    passes.clear();
    passParams.clear();
    colorTargets.clear();

    rendTarg.reset();
    outTex.DeleteIfValid();
}

RenderTarget* PostProcessing::Apply(const MTexture2D& worldColor,
                                    const MTexture2D& worldDepth)
{
    outTex.ClearData(worldColor.GetWidth(), worldColor.GetHeight());
    rendTarg->UpdateSize();

    //Generate dummy rendering info.
    Camera cam;
    Matrix4f identity;
    RenderInfo info(0.0f, &cam, &identity, &identity);
    
    RenderingState(RenderingState::C_NONE, false, false).EnableState();

    //Right now, we just have one pass that linearizes from HDR.
    rendTarg->EnableDrawingInto();
    ScreenClearer().ClearScreen();
    passParams[0]["u_tex"].Tex() = worldColor.GetTextureHandle();
    DrawingQuad::GetInstance()->Render(info, passParams[0], *passes[0]);
    rendTarg->DisableDrawingInto();


    return rendTarg.get();
}
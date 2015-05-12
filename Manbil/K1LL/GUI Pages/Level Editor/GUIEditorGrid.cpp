#include "GUIEditorGrid.h"

#include "../../../Rendering/Rendering.hpp"
#include "../../../Rendering/GUI/GUIMaterials.h"



GUIEditorGrid::GUIEditorGrid(Box2D& worldBnds, MTexture2D* _noiseTex, std::string& err)
    : WorldViewBounds(worldBnds), noiseTex(_noiseTex)
{
    //Vertex shader.
    RenderIOAttributes vIns = DrawingQuad::GetVertexInputData();
    std::string vOuts =
"out vec2 fIn_WorldPos; \n\
out vec2 fIn_UV;";
    MaterialUsageFlags vFlags;
    vFlags.EnableFlag(MaterialUsageFlags::DNF_USES_WVP_MAT);
    std::string vShaderHeader = MaterialConstants::GetVertexHeader(vOuts, vIns, vFlags);
    std::string wvp = MaterialConstants::WVPMatName,
                vIn_Pos = vIns.GetAttribute(0).Name,
                vIn_UV = vIns.GetAttribute(1).Name;

    std::string vShader = vShaderHeader + " \n\
uniform vec2 u_WorldMin, u_WorldMax; \n\
\n\
void main() \n\
{ \n\
    gl_Position = " + wvp + " * vec4(" + vIn_Pos + ", 1.0); \n\
    fIn_WorldPos = mix(u_WorldMin, u_WorldMax, 0.5 + (0.5 * " + vIn_Pos + ".xy)); \n\
    fIn_UV = 0.5 + (0.5 * " + vIn_Pos + ".xy); \n\
}";


    //Fragment shader.
    std::string fIns =
"in vec2 fIn_WorldPos; \n\
in vec2 fIn_UV;";
    std::string fOuts = "out vec4 fOut_Color;";
    std::string fShaderHeader = MaterialConstants::GetFragmentHeader(fIns, fOuts, MaterialUsageFlags());
    std::string colorParam = GUIMaterials::QuadDraw_Color;

    std::string fShader = fShaderHeader + " \n\
uniform sampler2D u_NoiseTex; \n\
uniform vec4 " + colorParam + "; \n\
\n\
void main() \n\
{ \n\
    //Draw a fuzzy line near every grid line and the zero of either axis. \n\
    \n\
    //Get the distance from the fragment to either the X or Y axis line (whichever is closer). \n\
    //Then, map it to a value from 0 to 1 indicating how close to the axis line the fragment is. \n\
    const float originDropoff = 4.5; \n\
    float distToOriginAxis = min(fIn_WorldPos.x, fIn_WorldPos.y), \n\
          originStrength = max(0.0, pow(1.0 - min(distToOriginAxis, 1.0), \n\
                                        originDropoff)); \n\
    \n\
    //Now calculate the same thing but for every grid line (along every integer coordinate). \n\
    const float gridDropoff = 6.5; \n\
    vec2 distToGridLine = 2.0 * abs(fract(abs(fIn_WorldPos)) - 0.5); \n\
    float distToGridLineAxis = max(distToGridLine.x, distToGridLine.y); \n\
    float gridStrength = pow(distToGridLineAxis, gridDropoff); \n\
    \n\
    //Combine the two calculated lines, as well as the blend color and noise texture. \n\
    float finalStrength = min(1.0, originStrength + gridStrength); \n\
    fOut_Color = vec4(vec3(finalStrength), 1.0) * " + colorParam + " * texture2D(u_NoiseTex, fIn_UV); \n\
}";
    

    //Compile the material.

    Params.Floats["u_WorldMin"] = UniformValueF(Vector2f(), "u_WorldMin");
    Params.Floats["u_WorldMax"] = UniformValueF(Vector2f(1.0f, 1.0f), "u_WorldMax");
    Params.Texture2Ds["u_NoiseTex"] = UniformValueSampler2D(0, "u_NoiseTex");
    Params.Floats[colorParam] = UniformValueF(Vector4f(1.0f, 1.0f, 1.0f, 1.0f), colorParam);

    Mat = new Material(vShader, fShader, Params, vIns, BlendMode::GetTransparent(), err);
    if (!err.empty())
    {
        err = "Error creating grid material: " + err;
        return;
    }
}

GUIEditorGrid::~GUIEditorGrid(void)
{
    delete Mat;
}

void GUIEditorGrid::Render(float elapsedTime, const RenderInfo& info)
{
    Params.Floats["u_WorldMin"].SetValue(WorldViewBounds.GetMinCorner());
    Params.Floats["u_WorldMax"].SetValue(WorldViewBounds.GetMaxCorner());
    Params.Texture2Ds["u_NoiseTex"].Texture = noiseTex->GetTextureHandle();

    GUITexture::Render(elapsedTime, info);
}
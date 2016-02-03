#include "ParticleManager.h"

#include <iostream>

#include "../../../Rendering/Primitives/DrawingQuad.h"

#include "../Level/Level.h"
#include "../../Content/ParticleContent.h"
#include "../../Content/QualitySettings.h"



//Stuff for the material that copies texture data to a new location.
namespace
{
    //Tries to generate a material that outputs texture data.
    Material* GenerateTexCopyMat(UniformDictionary& outParams, std::string& outErrorMsg)
    {
        //Generate the shaders.

        RenderIOAttributes vertIns = DrawingQuad::GetVertexInputData();
        std::string posName = vertIns.GetAttribute(0).Name,
                    uvName = vertIns.GetAttribute(1).Name;

        std::string vertShader = MaterialConstants::GetVertexHeader("out float fIn_UVx;\n", vertIns,
                                                                    MaterialUsageFlags());
        vertShader +=
R"(
uniform float u_texel_min, u_texel_max;

void main()
{
    fIn_UVx = mix(u_texel_min, u_texel_max, )" + uvName + R"( .x);
    gl_Position = vec4( )" + posName + R"( , 1.0);
}
)";

        std::string fragShader = MaterialConstants::GetFragmentHeader("in float fIn_UVx;\n",
                                                                      "out vec4 fOut_Tex1, fOut_Tex2;\n",
                                                                      MaterialUsageFlags());
        fragShader +=
R"(
uniform sampler2D u_tex1, u_tex2;

void main()
{
    vec2 uv = vec2(fIn_UVx, 0.0);
    fOut_Tex1 = texture2D(u_tex1, uv);
    fOut_Tex2 = texture2D(u_tex2, uv);
}
)";
        
        //Initialize the parameters.

        outParams["u_texel_min"] = Uniform("u_texel_min", UT_VALUE_F);
        outParams["u_texel_min"].Float().SetValue(0.0f);
        outParams["u_texel_max"] = Uniform("u_texel_max", UT_VALUE_F);
        outParams["u_texel_max"].Float().SetValue(0.0f);

        outParams["u_tex1"] = Uniform("u_tex1", UT_VALUE_SAMPLER2D);
        outParams["u_tex2"] = Uniform("u_tex2", UT_VALUE_SAMPLER2D);


        //Create and return the material.

        Material* mat = new Material(vertShader, fragShader, outParams, vertIns,
                                     BlendMode::GetOpaque(), outErrorMsg);
        if (!outErrorMsg.empty())
        {
            delete mat;
            mat = 0;
        }

        return mat;
    }

    void SetTexCopyMatParams(UniformDictionary& outParams, float texelMin, float texelMax,
                             MTexture2D& particleData1, MTexture2D& particleData2)
    {
        outParams["u_texel_min"].Float().SetValue(texelMin);
        outParams["u_texel_max"].Float().SetValue(texelMax);
        outParams["u_tex1"].Tex() = particleData1.GetTextureHandle();
        outParams["u_tex2"].Tex() = particleData2.GetTextureHandle();
    }
}


ParticleManager* ParticleManager::instance = 0;
std::string ParticleManager::initErrorMsg = "";

const std::string& ParticleManager::CreateInstance(Level* lvl)
{
    initErrorMsg.clear();

    //The constructor automatically sets the static "instance" field to point to it.
    new ParticleManager(lvl);

    if (initErrorMsg.empty())
    {
        //Just like any other Actor, this particle manager's memory is managed by the level.
        lvl->Actors.push_back(ActorPtr(instance));
    }
    else
    {
        //The destructor automatically resets "instance" to point to null.
        delete instance;
    }

    return initErrorMsg;
}

ParticleManager::ParticleManager(Level* lvl)
    : rendTarg(PS_16U_DEPTH, initErrorMsg), Actor(lvl),
      randVals(252341)
{
    assert(instance == 0);
    instance = this;

    if (!initErrorMsg.empty())
    {
        return;
    }

    //Set up particle data textures.
    for (unsigned int i = 0; i < 2; ++i)
    {
        particleData1[i].Create();
        particleData1[i].ClearData(QualitySettings::Instance.MaxParticles, 1);
        particleData2[i].Create();
        particleData2[i].ClearData(QualitySettings::Instance.MaxParticles, 1);
    }

    texelSize = 1.0f / (float)(QualitySettings::Instance.MaxParticles);


    //Set up the mesh for rendering particles.
    
    std::vector<RenderPassVertex> vertices;
    for (unsigned int i = 0; i < QualitySettings::Instance.MaxParticles; ++i)
        vertices.push_back(RenderPassVertex((float)i));
    
    burstRenderMesh.SubMeshes.push_back(MeshData(false, PT_POINTS));
    burstRenderMesh.SubMeshes[0].SetVertexData(vertices, MeshData::BUF_STATIC,
                                               RenderPassVertex::VertexAttrs);


    //Set up the material for copying particle data.
    copyParticleDataMat = GenerateTexCopyMat(copyParticleDataParams, initErrorMsg);
}
ParticleManager::~ParticleManager(void)
{
    delete copyParticleDataMat;

    assert(instance == this);
    instance = 0;
}


unsigned int ParticleManager::Burst(unsigned int nParticles, ParticleMaterial* mat, float lifetime,
                                     Vector3f sourceVel)
{
    //The bursts are always in order, so start this burst after the end of the last one.
    unsigned int startX = (bursts.size() == 0 ?
                               0 :
                               (bursts[bursts.size() - 1].StartPixel + bursts[bursts.size() - 1].NParticles));
    
    //Clip the number of particles so that there aren't more than the allowed max.
    if (startX >= QualitySettings::Instance.MaxParticles)
    {
        return 0;
    }
    nParticles = Mathf::Min(nParticles, QualitySettings::Instance.MaxParticles - startX);


    unsigned int renderFrom = currentRenderTo,
                 renderTo = (currentRenderTo + 1) % 2;


    //Set up the render target to use the particle data buffers.

    RenderTargetTex colTexes[] = { RenderTargetTex(&particleData1[renderTo]),
                                   RenderTargetTex(&particleData2[renderTo]) };
    rendTarg.SetColorAttachments(colTexes, 2, true);
    assert(rendTarg.GetWidth() == QualitySettings::Instance.MaxParticles);
    assert(rendTarg.GetHeight() == 1);

    Viewport::EnableScissor();
    Viewport(startX, 0, nParticles, 1).Use();

    RenderingState(RenderingState::C_NONE, false, false).EnableState();
    ScreenClearer().ClearScreen();


    //Note that the vertex shader for particle "burst" materials
    //    ignores the camera data and transform matrices.
    Camera c;
    Matrix4f identityM;
    RenderInfo rInfo(0.0f, &c, &identityM, &identityM, identityM);

    //Burst.
    float texelStart = (float)startX * texelSize,
          texelEnd = (float)(startX + nParticles - 1) * texelSize;
    ParticleContent::Instance.SetBurstPassParams(mat->BurstParams, texelStart, texelEnd,
                                                 randVals.GetZeroToOne(), sourceVel);
    std::cout << texelStart << ", " << texelEnd << "\n";
    DrawingQuad::GetInstance()->Render(rInfo, mat->BurstParams, *mat->BurstMat);
    bursts.push_back(ParticleBurst(mat, lifetime, startX, nParticles));


    //Reset the rendering state.
    Viewport::DisableScissor();
    rendTarg.DisableDrawingInto();


    return nParticles;
}

bool ParticleManager::Update(float elapsedSeconds)
{
    unsigned int renderFrom = (currentRenderTo + 1) % 2,
                 renderTo = currentRenderTo;


    //Set up a render target to use the particle data buffers.

    RenderTargetTex colTexes[] = { RenderTargetTex(&particleData1[renderTo]),
                                   RenderTargetTex(&particleData2[renderTo]) };
    rendTarg.SetColorAttachments(colTexes, 2, true);
    assert(rendTarg.GetWidth() == QualitySettings::Instance.MaxParticles);
    assert(rendTarg.GetHeight() == 1);

    Viewport::EnableScissor();

    RenderingState(RenderingState::C_NONE, false, false).EnableState();
    ScreenClearer().ClearScreen();


    //Run each burst's "update" material and check whether it should be destroyed.

    //Note that the vertex shader for particle "update" materials
    //    ignores the camera data and transform matrices.
    Camera c;
    Matrix4f identityM;

    for (unsigned int i = 0; i < bursts.size(); ++i)
    {
        RenderInfo rInfo(bursts[i].ElapsedTime, &c, &identityM, &identityM, identityM);

        if (Update(bursts[i], rInfo, elapsedSeconds, renderFrom))
        {
            std::pair<unsigned int, unsigned int> holeData(bursts[i].StartPixel, bursts[i].NParticles);
            holeStartsAndLengths.push_back(holeData);
         
            bursts.erase(bursts.begin() + i);
            i -= 1;
        }
    }
    
    //If no particles were destroyed this frame, switch the active particle textures like normal.
    if (holeStartsAndLengths.empty())
    {
        currentRenderTo = (currentRenderTo + 1) % 2;
    }
    //Otherwise, remove holes in the data buffers by outputting the data back to the first set of textures,
    //    with different UV's.
    else
    {
        renderTo = renderFrom;
        renderFrom = currentRenderTo;


        //Set up rendering state.

        Viewport::EnableScissor();

        colTexes[0].MTex = &particleData1[renderTo];
        colTexes[1].MTex = &particleData2[renderTo];
        rendTarg.SetColorAttachments(colTexes, 2, true);
        assert(rendTarg.GetWidth() == QualitySettings::Instance.MaxParticles);
        assert(rendTarg.GetHeight() == 1);
        
        //rendTarg.EnableDrawingInto();

        RenderingState(RenderingState::C_NONE, false, false).EnableState();
        ScreenClearer().ClearScreen();


        //Move each burst backwards in the texture to fill the holes.
        for (unsigned int i = 0; i < bursts.size(); ++i)
        {
            unsigned int moveAmount = 0;
            ParticleBurst& b = bursts[i];

            //Calculate how much this burst has to move backwards in the texture data.
            for (unsigned int j = 0; j < holeStartsAndLengths.size(); ++j)
            {
                unsigned int holeStart = holeStartsAndLengths[j].first,
                             holeLength = holeStartsAndLengths[j].second;

                assert(b.StartPixel != holeStart);
                if (b.StartPixel > holeStart)
                {
                    assert((holeStart + holeLength - 1) < b.StartPixel);
                    moveAmount += holeLength;
                }
            }
            assert(b.StartPixel >= moveAmount);

            //Now move it backwards.
            MoveData(renderFrom, b.StartPixel, b.StartPixel - moveAmount, b.NParticles);
            b.StartPixel -= moveAmount;
        }

        holeStartsAndLengths.clear();
    }
    

    //Reset the rendering state.
    Viewport::DisableScissor();
    rendTarg.DisableDrawingInto();


    //This actor should live for the entire game.
    return false;
}
bool ParticleManager::Update(ParticleBurst& burst, const RenderInfo& info,
                              float frameLength, unsigned int renderFrom)
{
    unsigned int endPixel = burst.StartPixel + burst.NParticles - 1;

    ParticleContent::Instance.SetUpdatePassParams(burst.Mat->UpdateParams,
                                                  (float)burst.StartPixel * texelSize,
                                                  (float)endPixel * texelSize,
                                                  randVals.GetZeroToOne(), frameLength,
                                                  &particleData1[renderFrom], &particleData2[renderFrom]);

    Viewport(burst.StartPixel, 0, burst.NParticles, 1).Use();

    DrawingQuad::GetInstance()->Render(info, burst.Mat->UpdateParams, *burst.Mat->UpdateMat);

    burst.ElapsedTime += frameLength;
    return (burst.ElapsedTime >= burst.Lifetime);
}

void ParticleManager::MoveData(unsigned int renderFrom, unsigned int srcStart, unsigned int destStart,
                                unsigned int nParticles)
{
    unsigned int srcEnd = srcStart + nParticles - 1,
                 destEnd = destStart + nParticles - 1;

    Viewport(destStart, 0, nParticles, 1).Use();

    Camera cam;
    Matrix4f identityM;
    RenderInfo inf(0.0f, &cam, &identityM, &identityM, identityM);

    SetTexCopyMatParams(copyParticleDataParams, (float)srcStart * texelSize, (float)srcEnd * texelSize,
                        particleData1[renderFrom], particleData2[renderFrom]);

    DrawingQuad::GetInstance()->Render(inf, copyParticleDataParams, *copyParticleDataMat);
}

void ParticleManager::Render(float elapsedSeconds, const RenderInfo& info)
{
    unsigned int renderFrom = (currentRenderTo + 1) % 2;

    for (unsigned int i = 0; i < bursts.size(); ++i)
    {
        Render(bursts[i], info, renderFrom);
    }
}
void ParticleManager::Render(const ParticleBurst& burst, const RenderInfo& info, unsigned int renderFrom)
{
    Material* mat = burst.Mat->RenderMat;
    UniformDictionary& params = burst.Mat->RenderParams;

    ParticleContent::Instance.SetRenderPassParams(params, texelSize,
                                                  (float)burst.StartPixel * texelSize,
                                                  &particleData1[renderFrom], &particleData2[renderFrom]);
    burstRenderMesh.SubMeshes[0].SetUseRange(0, burst.NParticles);
    mat->Render(info, &burstRenderMesh, params);
}
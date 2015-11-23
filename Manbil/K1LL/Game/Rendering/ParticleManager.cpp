#include "ParticleManager.h"

#include <iostream>

#include "../../../Rendering/Primitives/DrawingQuad.h"

#include "../Level/Level.h"
#include "../../Content/ParticleContent.h"
#include "../../Content/QualitySettings.h"


namespace
{
    //The number of particle sets that can be fit along one axis of the texture.
    const unsigned int NParticleSets[3] =
    {
        ParticleManager::TextureSize[0] / ParticleManager::ParticlesLength[0],
        ParticleManager::TextureSize[1] / ParticleManager::ParticlesLength[1],
        ParticleManager::TextureSize[2] / ParticleManager::ParticlesLength[2],
    };
    const float TexelSize[3] =
    {
        1.0f / ParticleManager::TextureSize[0],
        1.0f / ParticleManager::TextureSize[1],
        1.0f / ParticleManager::TextureSize[2],
    };

    std::string errMsg;
}

//Particles are stored in square regions on a set of textures.
//This holds the number of particles along each side for the three different sizes.
const unsigned int ParticleManager::ParticlesLength[3] =
{
    8,
    32,
    64,
};
//The size of each texture used to store all particles of a certain size.
const unsigned int ParticleManager::TextureSize[3] =
{
    64,
    256,
    512,
};


ParticleManager* ParticleManager::instance = 0;


void ParticleManager::InitializeInstance(Level* lvl)
{
    //Note that the manager's memory is entirely managed by the level.
    instance = new ParticleManager(lvl);
    lvl->Actors.push_back(ActorPtr(instance));
}


ParticleManager::ParticleManager(Level* lvl)
    : updateRendTarg(PS_16U_DEPTH, errMsg),
      Actor(lvl)
{
    if (!errMsg.empty())
    {
        std::cout << "Error initializing particle manager's render target: " << errMsg << "\n";
        assert(false);
        errMsg.clear();
    }

    //Set up the particle sets.
    for (unsigned int i = S_SMALL; i <= S_LARGE; ++i)
    {
        particlesInTexture[i].Reset(NParticleSets[i], NParticleSets[i],
                                    ParticleManager::ParticleSet(0, 0.0f, Vector2u(), (Sizes)i));
    }


    //Set up the textures.
    for (unsigned int i = 0; i < 2; ++i)
    {
        for (unsigned int j = S_SMALL; j <= S_LARGE; ++j)
        {
            Textures[i][j].first.Create();
            Textures[i][j].first.ClearData(TextureSize[j], TextureSize[j]);
            Textures[i][j].second.Create();
            Textures[i][j].second.ClearData(TextureSize[j], TextureSize[j]);
        }
    }

    //Set up the particle mesh.
    particleMesh.SubMeshes.push_back(MeshData(false, PT_POINTS));
    MeshData& m = particleMesh.SubMeshes[0];
    std::vector<RenderPassVertex> vertices;

#define ADD_RANGE(xMin, xMax, yMin, yMax) \
    for (unsigned int x = xMin; x < xMax; ++x) \
        for (unsigned int y = yMin; y < yMax; ++y) \
            vertices.push_back(RenderPassVertex(Vector2f((float)x, (float)y)));
    //First generate all the particles for small bursts.
    ADD_RANGE(0, ParticlesLength[0], 0, ParticlesLength[0]);
    //Next, add more particles to fill out medium bursts.
    ADD_RANGE(ParticlesLength[0], ParticlesLength[1], 0, ParticlesLength[1]);
    ADD_RANGE(0, ParticlesLength[0], ParticlesLength[0], ParticlesLength[1]);
    //Finally, generate the rest of the particles to fill out large bursts.
    ADD_RANGE(ParticlesLength[1], ParticlesLength[2], 0, ParticlesLength[2]);
    ADD_RANGE(0, ParticlesLength[1], ParticlesLength[1], ParticlesLength[2]);

    m.SetVertexData(vertices, MeshData::BUF_STATIC, RenderPassVertex::VertexAttrs);
}

bool ParticleManager::Update(float frameSeconds)
{
    Camera cam;
    Matrix4f viewM, projM;
    RenderInfo info(0.0f, &cam, &viewM, &projM);
    
    unsigned int pongPing = (pingPong + 1) % 2;

    for (unsigned int i = S_SMALL; i <= S_LARGE; ++i)
    {
        ParticleTextures& renderTo = Textures[pingPong][i];
        ParticleTextures* sampleFrom = &Textures[pongPing][i];

        //Update all active particle sets for this size.
        for (Vector2u count; count.y < particlesInTexture[i].GetHeight(); ++count.y)
        {
            for (count.x = 0; count.x < particlesInTexture[i].GetWidth(); ++count.x)
            {
                ParticleSet& particles = particlesInTexture[i][count];
                if (particles.Mat != 0)
                {
                    info.TotalElapsedSeconds = particles.ElapsedTime;

                    RunUpdate((ParticleManager::Sizes)i,
                              particles.Mat->UpdateMat, particles.Mat->UpdateParams,
                              info, false, count, renderTo, frameSeconds, sampleFrom);

                    particles.ElapsedTime += frameSeconds;
                    if (particles.ElapsedTime >= particles.Lifetime)
                    {
                        particles.Mat = 0;
                    }
                }
            }
        }
    }

    pingPong = pongPing;

    return false;
}
void ParticleManager::Render(float frameSeconds, const RenderInfo& info)
{
    for (unsigned int i = S_SMALL; i <= S_LARGE; ++i)
    {
        ParticleTextures& sampleFrom = Textures[(pingPong + 1) % 2][i];

        //Update all active particle sets for this size.
        for (Vector2u count; count.y < particlesInTexture[i].GetHeight(); ++count.y)
        {
            for (count.x = 0; count.x < particlesInTexture[i].GetWidth(); ++count.x)
            {
                ParticleSet& particles = particlesInTexture[i][count];
                if (particles.Mat != 0)
                {
                    RunRender((ParticleManager::Sizes)i, info, count, *particles.Mat, sampleFrom);
                }
            }
        }
    }
}

void ParticleManager::Burst(Sizes burstSize, ParticleMaterial* material, float lifetime)
{
    //Find the first unused part of the particle texture data to reserve.
    Vector2u toUse;
    while (toUse.y < particlesInTexture[burstSize].GetHeight() &&
           particlesInTexture[burstSize][toUse].Mat != 0)
    {
        toUse.x += 1;
        if (toUse.x >= particlesInTexture[burstSize].GetWidth())
        {
            toUse.x = 0;
            toUse.y += 1;
        }
    }

    particlesInTexture[burstSize][toUse] = ParticleSet(material, lifetime,
                                                       toUse * ParticlesLength[burstSize],
                                                       burstSize);

    //Run the burst material.
    Camera cam;
    Matrix4f viewM, projM;
    RenderInfo info(0.0f, &cam, &viewM, &projM);
    RunUpdate(burstSize, material->BurstMat, material->BurstParams, info,
              true, toUse, Textures[(pingPong + 1) % 2][burstSize]);
}

void ParticleManager::RunUpdate(Sizes size, Material* mat, UniformDictionary& params, const RenderInfo& in,
                                bool isBurst, Vector2u particleSetIndex, ParticleTextures& renderTo,
                                float timeStep, ParticleTextures* sampleFrom)
{
    Vector2u startPixel = particleSetIndex * ParticlesLength[size],
             endPixel = startPixel + Vector2u(ParticlesLength[size], ParticlesLength[size]);
    ParticleContent::Instance.SetUpdatePassParams(params, isBurst,
                                                  ToV2f(startPixel) * TexelSize[size],
                                                  ToV2f(endPixel) * TexelSize[size],
                                                  timeStep,
                                                  (isBurst ? 0 : &sampleFrom->first),
                                                  (isBurst ? 0 : &sampleFrom->second));

    //TODO: Pull out all render target and scissor stuff below into the code that calls this function.

    const RenderTarget* oldTarg = RenderTarget::GetCurrentTarget();


    RenderTargetTex texes[2] = { RenderTargetTex(&renderTo.first), RenderTargetTex(&renderTo.second) };
    updateRendTarg.SetColorAttachments(texes, 2, true);

    assert(updateRendTarg.GetWidth() == TextureSize[size] &&
           updateRendTarg.GetHeight() == TextureSize[size]);

    Viewport::EnableScissor();
    Viewport v(startPixel.x, startPixel.y, ParticlesLength[size], ParticlesLength[size]);
    updateRendTarg.EnableDrawingInto(v);
    v.SetAsScissor();

    ScreenClearer().ClearScreen();
    RenderingState(RenderingState::C_NONE, false, false).EnableState();

    DrawingQuad::GetInstance()->Render(in, params, *mat);

    updateRendTarg.DisableDrawingInto();
    Viewport::DisableScissor();

    if (oldTarg != 0)
    {
        oldTarg->EnableDrawingInto();
    }
}
void ParticleManager::RunRender(Sizes size, const RenderInfo& info, Vector2u particleSetIndex,
                                ParticleMaterial& mat, ParticleTextures& sampleFrom)
{
    ParticleContent::Instance.SetRenderPassParams(mat.RenderParams, TexelSize[size],
                                                  ToV2f(particleSetIndex) * TexelSize[size],
                                                  &sampleFrom.first, &sampleFrom.second);

    particleMesh.SubMeshes[0].SetUseRange(0, ParticlesLength[size] * ParticlesLength[size]);
    mat.RenderMat->Render(info, &particleMesh, mat.RenderParams);
}




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


ParticleManager2* ParticleManager2::instance = 0;

const std::string& ParticleManager2::CreateInstance(Level* lvl)
{
    initErrorMsg.clear();

    //The constructor automatically sets the static "instance" field to point to it.
    new ParticleManager2(lvl);

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

ParticleManager2::ParticleManager2(Level* lvl)
    : rendTarg(PS_16U_DEPTH, initErrorMsg), Actor(lvl)
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
        particleData1[i].ClearData(QualitySettings::Instance.MaxParticles, 2);
        particleData2[i].Create();
        particleData2[i].ClearData(QualitySettings::Instance.MaxParticles, 2);
    }

    texelSize = 1.0f / (float)(QualitySettings::Instance.MaxParticles);


    //Set up the mesh for rendering particles.
    
    std::vector<RenderPassVertex> vertices;
    for (unsigned int i = 0; i < QualitySettings::Instance.MaxParticles; ++i)
        vertices.push_back(RenderPassVertex(Vector2f((float)i, 0.0f)));
    
    burstRenderMesh.SubMeshes.push_back(MeshData(false, PT_POINTS));
    burstRenderMesh.SubMeshes[0].SetVertexData(vertices, MeshData::BUF_STATIC,
                                               RenderPassVertex::VertexAttrs);


    //Set up the material for copying particle data.
    copyParticleDataMat = GenerateTexCopyMat(copyParticleDataParams, initErrorMsg);
}
ParticleManager2::~ParticleManager2(void)
{
    delete copyParticleDataMat;

    assert(instance == this);
    instance = 0;
}


unsigned int ParticleManager2::Burst(unsigned int nParticles, ParticleMaterial* mat, float lifetime)
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

    const RenderTarget* oldTarg = RenderTarget::GetCurrentTarget();

    RenderTargetTex colTexes[] = { RenderTargetTex(&particleData1[renderTo]),
                                   RenderTargetTex(&particleData2[renderTo]) };
    rendTarg.SetColorAttachments(colTexes, 2, true);
    assert(rendTarg.GetWidth() == QualitySettings::Instance.MaxParticles);
    assert(rendTarg.GetHeight() == 1);

    Viewport::EnableScissor();
    rendTarg.EnableDrawingInto();

    Viewport(startX, 0, nParticles, 1).Use();

    ScreenClearer().ClearScreen();
    RenderingState(RenderingState::C_NONE, false, false).EnableState();


    //Note that the vertex shader for particle "burst" materials
    //    ignores the camera data and transform matrices.
    Camera c;
    Matrix4f identityM;
    RenderInfo rInfo(0.0f, &c, &identityM, &identityM, identityM);

    //Burst.
    ParticleContent::Instance.SetUpdatePassParams(mat->BurstParams, true,
                                                  Vector2f((float)startX * texelSize, 0.0f),
                                                  Vector2f((float)(startX + nParticles - 1), 0.0f));
    DrawingQuad::GetInstance()->Render(rInfo, mat->BurstParams, *mat->BurstMat);
    bursts.push_back(ParticleBurst(mat, lifetime, startX, nParticles));


    //Reset the rendering state.
    Viewport::DisableScissor();
    rendTarg.DisableDrawingInto();
    if (oldTarg != 0)
    {
        oldTarg->EnableDrawingInto();
    }


    return nParticles;
}

bool ParticleManager2::Update(float elapsedSeconds)
{
    unsigned int renderFrom = (currentRenderTo + 1) % 2,
                 renderTo = currentRenderTo;


    //Set up a render target to use the particle data buffers.

    const RenderTarget* oldTarg = RenderTarget::GetCurrentTarget();

    RenderTargetTex colTexes[] = { RenderTargetTex(&particleData1[renderTo]),
                                   RenderTargetTex(&particleData2[renderTo]) };
    rendTarg.SetColorAttachments(colTexes, 2, true);
    assert(rendTarg.GetWidth() == QualitySettings::Instance.MaxParticles);
    assert(rendTarg.GetHeight() == 1);

    Viewport::EnableScissor();
    rendTarg.EnableDrawingInto();

    ScreenClearer().ClearScreen();
    RenderingState(RenderingState::C_NONE, false, false).EnableState();


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
        
        rendTarg.EnableDrawingInto();

        ScreenClearer().ClearScreen();
        RenderingState(RenderingState::C_NONE, false, false).EnableState();


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
    //TODO: Remove this resetting of render target; code that runs during Update() shouldn't effect code that runs during Render(). Also remove it from Burst().
    rendTarg.DisableDrawingInto();
    if (oldTarg != 0)
    {
        oldTarg->EnableDrawingInto();
    }


    //This actor should live for the entire game.
    return false;
}
bool ParticleManager2::Update(ParticleBurst& burst, const RenderInfo& info,
                              float frameLength, unsigned int renderFrom)
{
    unsigned int endPixel = burst.StartPixel + burst.NParticles - 1;

    ParticleContent::Instance.SetUpdatePassParams(burst.Mat->UpdateParams, false,
                                                  Vector2f((float)burst.StartPixel * texelSize, 0.0f),
                                                  Vector2f((float)endPixel * texelSize, 0.0f),
                                                  frameLength,
                                                  &particleData1[renderFrom], &particleData2[renderFrom]);

    Viewport(burst.StartPixel, 0, burst.NParticles, 1).Use();

    DrawingQuad::GetInstance()->Render(info, burst.Mat->UpdateParams, *burst.Mat->UpdateMat);

    burst.ElapsedTime += frameLength;
    return (burst.ElapsedTime >= burst.Lifetime);
}

void ParticleManager2::MoveData(unsigned int renderFrom, unsigned int srcStart, unsigned int destStart,
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

void ParticleManager2::Render(float elapsedSeconds, const RenderInfo& info)
{
    unsigned int renderFrom = (currentRenderTo + 1) % 2;

    for (unsigned int i = 0; i < bursts.size(); ++i)
    {
        Render(bursts[i], info, renderFrom);
    }
}
void ParticleManager2::Render(const ParticleBurst& burst, const RenderInfo& info, unsigned int renderFrom)
{
    Material* mat = burst.Mat->RenderMat;
    UniformDictionary& params = burst.Mat->RenderParams;

    ParticleContent::Instance.SetRenderPassParams(params, texelSize,
                                                  Vector2f((float)burst.StartPixel * texelSize, 0.0f),
                                                  &particleData1[renderFrom], &particleData2[renderFrom]);
    burstRenderMesh.SubMeshes[0].SetUseRange(0, burst.NParticles);
    mat->Render(info, &burstRenderMesh, params);
}
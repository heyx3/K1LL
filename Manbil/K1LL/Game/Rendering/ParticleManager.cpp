#include "ParticleManager.h"

#include <iostream>

#include "../../../Rendering/Primitives/DrawingQuad.h"

#include "../Level/Level.h"
#include "../../Content/ParticleContent.h"


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
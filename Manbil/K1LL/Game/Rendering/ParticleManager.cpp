#include "ParticleManager.h"

#include <iostream>

#include "../Level/Level.h"


namespace
{
    //The number of particle sets that can be fit along one axis of the texture.
    const unsigned int NParticleSets[3] =
    {
        ParticleManager::TextureSize[0] / ParticleManager::ParticlesLength[0],
        ParticleManager::TextureSize[1] / ParticleManager::ParticlesLength[1],
        ParticleManager::TextureSize[2] / ParticleManager::ParticlesLength[2],
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


    //Set up the textures.
    for (int i = 0; i < 2; ++i)
    {
        for (int j = S_SMALL; j <= S_LARGE; ++j)
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
                    RunUpdate((ParticleManager::Sizes)i,
                              particles.Mat->UpdateMat, particles.Mat->UpdateParams,
                              info, false, renderTo, sampleFrom);
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
                    RunRender((ParticleManager::Sizes)i, info, *particles.Mat, sampleFrom);
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
              true, Textures[pingPong][burstSize]);
}

void ParticleManager::RunUpdate(Sizes size, Material* mat, UniformDictionary& params, const RenderInfo& in,
                                bool isBurst, ParticleTextures& renderTo, ParticleTextures* sampleFrom)
{
    if (!isBurst)
    {
        params.Texture2Ds[ParticleMaterial::UNIFORM_TEX1].Texture = sampleFrom->first.GetTextureHandle();
        params.Texture2Ds[ParticleMaterial::UNIFORM_TEX2].Texture = sampleFrom->second.GetTextureHandle();
    }

    RenderTargetTex texes[2] = { RenderTargetTex(&renderTo.first), RenderTargetTex(&renderTo.second) };
    updateRendTarg.SetColorAttachments(texes, 2, true);

    updateRendTarg.EnableDrawingInto();

    glViewport(0, 0, TextureSize[size], TextureSize[size]);
    RenderingState(RenderingState::C_NONE, false, false).EnableState();
    ScreenClearer().ClearScreen();

    particleMesh.SubMeshes[0].SetUseRange(0, ParticlesLength[size] * ParticlesLength[size]);
    mat->Render(in, &particleMesh, params);

    updateRendTarg.DisableDrawingInto();
}
void ParticleManager::RunRender(Sizes size, const RenderInfo& info,
                                ParticleMaterial& mat, ParticleTextures& sampleFrom)
{
    mat.RenderParams.Texture2Ds[ParticleMaterial::UNIFORM_TEX1].Texture =
        sampleFrom.first.GetTextureHandle();
    mat.RenderParams.Texture2Ds[ParticleMaterial::UNIFORM_TEX2].Texture =
        sampleFrom.second.GetTextureHandle();

    particleMesh.SubMeshes[0].SetUseRange(0, ParticlesLength[size] * ParticlesLength[size]);
    mat.RenderMat->Render(info, &particleMesh, mat.RenderParams);
}
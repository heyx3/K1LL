#pragma once

#include "../../../Rendering/Rendering.hpp"

#include "../../Content/ParticleContent.h"
#include "../Actor.h"



namespace ParticleConstants
{
    //Particles are stored in square regions on a set of textures.
    //This holds the number of particles along each side for the three different sizes.
    static const unsigned int ParticlesLength[3] =
    {
        8,
        32,
        64,
    };
    //The size of each texture used to store all particles of a certain size.
    static const unsigned int TextureSize[3] =
    {
        64,
        256,
        512,
    };
}


//An actor that handles all particle updating/rendering.
//Only one instance exists at a time, and it's statically-accessible.
class ParticleManager : public Actor
{
public:

    //The three different particle burst sizes.
    enum Sizes
    {
        S_SMALL = 0,
        S_MEDIUM,
        S_LARGE,
    };


    static ParticleManager& GetInstance(void) { return *instance; }

    //Creates a new instance and puts it into the given level.
    static void InitializeInstance(Level* lvl);


    virtual bool Update(float elapsedSeconds) override;
    virtual void Render(float elapsedSeconds, const RenderInfo& info) override;

    void Burst(Sizes burstSize, ParticleMaterial* material, float lifetime);


private:

    //A single burst of active particles.
    struct ParticleSet
    {
        ParticleMaterial* Mat;
        float Lifetime;
        
        float ElapsedTime = 0.0f;

        Vector2u StartPixel;
        Sizes Size;


        ParticleSet(void) { }
        ParticleSet(ParticleMaterial* mat, float lifetime, Vector2u startPixel, Sizes size)
            : Mat(mat), Lifetime(lifetime), StartPixel(startPixel), Size(size) { }
    };

    //All textures needed for a set of particles.
    typedef std::pair<MTexture2D, MTexture2D> ParticleTextures;


    static ParticleManager* instance;


    //The currently-active small, medium, and large particle bursts.
    Array2D<ParticleSet> particlesInTexture[3];

    //Ping-pong between two different sets of textures for particle updating.
    //Have different sets of particle textures for each size.
    ParticleTextures Textures[2][3] =
    {
#define TEX_SETTINGS TextureSampleSettings2D(FT_NEAREST, WT_CLAMP), PS_32F, false
        {
            ParticleTextures(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)),
            ParticleTextures(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)),
            ParticleTextures(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)),
        },
        {
            ParticleTextures(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)),
            ParticleTextures(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)),
            ParticleTextures(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)),
        },
#undef TEX_SETTINGS
    };

    RenderTarget updateRendTarg;
    Mesh particleMesh;

    unsigned int pingPong = 0;


    ParticleManager(Level* lvl);


    //Runs an "update" or "burst" pass.
    void RunUpdate(Sizes particleSetSize, Material* mat, UniformDictionary& params, const RenderInfo& info,
                   bool isBurst, ParticleTextures& renderTo, ParticleTextures* sampleFrom = 0);
    //Runs a "render" pass.
    void RunRender(Sizes particleSize, const RenderInfo& info,
                   ParticleMaterial& mat, ParticleTextures& sampleFrom);
};
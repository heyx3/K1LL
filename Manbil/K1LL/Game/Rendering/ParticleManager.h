#pragma once

#include "../../../Rendering/Rendering.hpp"

#include "../../Content/ParticleContent.h"
#include "../Actor.h"



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


    //Particles are stored in square regions on a set of textures.
    //This holds the number of particles along each side for the three different sizes.
    static const unsigned int ParticlesLength[3];
    //The size of each texture used to store all particles of a certain size.
    static const unsigned int TextureSize[3];


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
        ParticleMaterial* Mat = 0;
        float Lifetime;
        
        float ElapsedTime = 0.0f;

        Vector2u StartPixel;
        Sizes Size;


        ParticleSet(void) { }
        ParticleSet(ParticleMaterial* mat, float lifetime, Vector2u startPixel, Sizes size)
            : Mat(mat), Lifetime(lifetime), StartPixel(startPixel), Size(size) { }
    };

    //A workaround because C++ doesn't allow anything other than default constructors for array elements.
    //How the fuck is that a thing??
    class ParticleSet2DArray : public Array2D<ParticleSet>
    {
    public:

        ParticleSet2DArray(void) : Array2D<ParticleSet>(1, 1) { }
    };
    //Another workaround for the "default constructor for array elements" issue.
    //The set of textures needed to fully describe particle data.
    class ParticleTextures : public std::pair<MTexture2D, MTexture2D>
    {
    public:
#define TEX_SETTINGS TextureSampleSettings2D(FT_NEAREST, WT_CLAMP), PS_32F, false
        ParticleTextures(void)
            : std::pair<MTexture2D, MTexture2D>(MTexture2D(TEX_SETTINGS), MTexture2D(TEX_SETTINGS)) { }
#undef TEX_SETTINGS
    };


    static ParticleManager* instance;


    //The currently-active small, medium, and large particle bursts.
    ParticleSet2DArray particlesInTexture[3];

    //Ping-pong between two different sets of textures for particle updating.
    //Have different sets of particle textures for each size.
    ParticleTextures Textures[2][3];

    RenderTarget updateRendTarg;
    Mesh particleMesh;

    unsigned int pingPong = 0;


    ParticleManager(Level* lvl);


    //Runs an "update" or "burst" pass.
    //"sampleFrom" and "frameLength" only apply to "update" passes, not "burst" passes.
    void RunUpdate(Sizes particleSetSize, Material* mat, UniformDictionary& params, const RenderInfo& info,
                   bool isBurst, Vector2u particleSetIndex, ParticleTextures& renderTo,
                   float frameLength = 0.0f, ParticleTextures* sampleFrom = 0);
    //Runs a "render" pass.
    void RunRender(Sizes particleSize, const RenderInfo& info, Vector2u particleSetIndex,
                   ParticleMaterial& mat, ParticleTextures& sampleFrom);
};
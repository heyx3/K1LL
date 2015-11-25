#pragma once

#include "../Actor.h"



//A set of "burst", "update", and "render" materials for a specific particle effect.
struct ParticleMaterial
{
    Material *BurstMat = 0,
             *UpdateMat = 0,
             *RenderMat = 0;
    UniformDictionary BurstParams, UpdateParams, RenderParams;
};


//A group of particles that were emitted together.
struct ParticleBurst
{
    ParticleMaterial* Mat = 0;
    float Lifetime;

    float ElapsedTime = 0.0f;

    unsigned int StartPixel;
    unsigned int NParticles;


    ParticleBurst(void) { }
    ParticleBurst(ParticleMaterial* mat, float lifeTime, unsigned int startPixel, unsigned int nParticles)
        : Mat(mat), Lifetime(lifeTime), StartPixel(startPixel), NParticles(nParticles) { }
};


/*
//An actor that handles all particle updating/rendering.
//Only one instance exists at a time, and it's accessible as a Singleton.
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
*/


//An actor that handles all particle updating/rendering.
//Only one instance exists at a time, and it's accessible as a Singleton.
class ParticleManager2 : public Actor
{
public:

    static ParticleManager2& GetInstance(void) { return *instance; }

    //Creates the instance of this singleton actor and puts it into the given level.
    //Returns an error message if something went wrong, or the empty string if everything went OK.
    static const std::string& CreateInstance(Level* lvl);


    ~ParticleManager2(void);


    //Attempts to emit the given number of particles using the given shaders.
    //Returns the number of particles that were actually emitted;
    //    this will be less than the given number if we hit the max number of particles allowed.
    unsigned int Burst(unsigned int nParticles, ParticleMaterial* material, float lifetime);

    virtual bool Update(float elapsedSeconds) override;
    virtual void Render(float elapsedSeconds, const RenderInfo& info) override;


private:

    static ParticleManager2* instance;
    static std::string initErrorMsg;

    
    //Any element in a stack-allocated array needs a default constructor.
    //This class just adds a default constructor to MTexture2D.
    class ParticleDataTex : public MTexture2D
    {
    public:
        ParticleDataTex(void)
            : MTexture2D(TextureSampleSettings2D(FT_NEAREST, WT_CLAMP), PS_16F, false) { }
    };

    //The particle data is stored inside two Nx1 textures.
    //Each particle has a specific X coordinate in the textures that it stores its data in.
    //Store two copies of the particle data textures and ping-pong between them.
    ParticleDataTex particleData1[2],
                    particleData2[2];
    unsigned int currentRenderTo = 0;

    //Used to directly copy particle data between the textures.
    Material* copyParticleDataMat;
    UniformDictionary copyParticleDataParams;

    //The horizontal size of a single texel in the particle data texture.
    float texelSize;

    RenderTarget rendTarg;
    Mesh burstRenderMesh;

    std::vector<ParticleBurst> bursts;

    //Used to track holes in the particle data textures as bursts are removed.
    std::vector<std::pair<unsigned int, unsigned int>> holeStartsAndLengths;

    ParticleManager2(Level* lvl);


    //Updates the given burst and returns whether it should be destroyed.
    bool Update(ParticleBurst& burst, const RenderInfo& info, float frameLength, unsigned int renderFrom);

    void Render(const ParticleBurst& burst, const RenderInfo& info, unsigned int renderFrom);

    //Outputs the given particles from the given particle data texture into the currently-bound render target,
    //    assumed to be the other particle data texture, with an offset UV.
    void MoveData(unsigned int renderFrom, unsigned int srcStart, unsigned int destStart,
                  unsigned int nParticles);
};
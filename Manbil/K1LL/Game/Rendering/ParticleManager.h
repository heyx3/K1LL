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



//An actor that handles all particle updating/rendering.
//Only one instance exists at a time, and it's accessible as a Singleton.
class ParticleManager : public Actor
{
public:

    static ParticleManager& GetInstance(void) { return *instance; }

    //Creates the instance of this singleton actor and puts it into the given level.
    //Returns an error message if something went wrong, or the empty string if everything went OK.
    static const std::string& CreateInstance(Level* lvl);


    ~ParticleManager(void);


    //Attempts to emit the given number of particles using the given shaders.
    //Returns the number of particles that were actually emitted;
    //    this will be less than the given number if we hit the max number of particles allowed.
    unsigned int Burst(unsigned int nParticles, ParticleMaterial* material, float lifetime,
                       Vector3f sourceVelocity);

    virtual bool Update(float elapsedSeconds) override;
    virtual void Render(float elapsedSeconds, const RenderInfo& info) override;


private:

    static ParticleManager* instance;
    static std::string initErrorMsg;

    
    //Any element in a stack-allocated array needs a default constructor.
    //This class just adds a default constructor to MTexture2D.
    class ParticleDataTex : public MTexture2D
    {
    public:
        ParticleDataTex(void)
            : MTexture2D(TextureSampleSettings2D(FT_NEAREST, WT_CLAMP), PS_32F, false) { }
    };


    //The particle data is stored inside two Nx1 textures.
    //Each particle has a specific X coordinate in the textures that it stores its data in.
    //Store two copies of the particle data textures and ping-pong between them.
    ParticleDataTex particleData1[2],
                    particleData2[2];
    unsigned int currentRenderTo = 0;

    FastRand randVals;

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


    ParticleManager(Level* lvl);


    //Updates the given burst and returns whether it should be destroyed.
    bool Update(ParticleBurst& burst, const RenderInfo& info, float frameLength, unsigned int renderFrom);

    void Render(const ParticleBurst& burst, const RenderInfo& info, unsigned int renderFrom);

    //Outputs the given particles from the given particle data texture into the currently-bound render target,
    //    assumed to be the other particle data texture, with an offset UV.
    void MoveData(unsigned int renderFrom, unsigned int srcStart, unsigned int destStart,
                  unsigned int nParticles);
};
#pragma once

#include "../../Rendering/Rendering.hpp"

#include "../Game/Rendering/ParticleManager.h"


//PRIORITY: Generate a noise texture for random values when bursting.



struct RenderPassVertex
{
    //Integer value indicating the particle's X coordinate offset in the texture data.
    float ID;
    RenderPassVertex(float id = 0.0f) : ID(id) { }

    static const RenderIOAttributes VertexAttrs;
};


//Handles bursting, updating, and rendering particles.
class ParticleContent
{
public:

    static ParticleContent Instance;

    
    MTexture2D RandTex;

    ParticleMaterial PuncherFire;


    //Returns whether initialization succeeded.
    bool Initialize(std::string& outError);
    void Destroy(void);

    //Sets the basic parameters for a burst pass.
    void SetBurstPassParams(UniformDictionary& params,
                             float particleTexelMin, float particleTexelMax,
                             float randVal, Vector3f sourceVelocity);
    //Sets the basic parameters for an update pass.
    void SetUpdatePassParams(UniformDictionary& params,
                             float particleTexelMin, float particleTexelMax,
                             float randVal, float timeStep = 0.0f,
                             MTexture2D* particleTexData1 = 0, MTexture2D* particleTexData2 = 0);
    //Sets up the basic uniforms used by the given render pass.
    void SetRenderPassParams(UniformDictionary& params,
                             float texelSize, float particleTexelMin,
                             MTexture2D* particleTexData1, MTexture2D* particleTexData2);


    void PuncherFire_Burst(Vector3f pos, Vector3f dir, Vector3f tangent, Vector3f bitangent,
                           Vector3f playerVelocity);


private:

    ParticleContent(void) : RandTex(TextureSampleSettings2D(FT_NEAREST, WT_WRAP), PS_8U, false) { }
};
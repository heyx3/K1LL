#pragma once

#include "../../Rendering/Rendering.hpp"

#include "../Game/Rendering/ParticleManager.h"


//PRIORITY: Fix to use updated particle texture data system. Generate a noise texture for random values when bursting.



struct RenderPassVertex
{
    //Integer values indicating the particle's pixel coordinate offset in the texture data.
    Vector2f ID;
    RenderPassVertex(Vector2f id = Vector2f()) : ID(id) { }

    static const RenderIOAttributes VertexAttrs;
};


//Handles bursting, updating, and rendering particles.
class ParticleContent
{
public:

    static ParticleContent Instance;


    ParticleMaterial PuncherFire;


    //Returns whether initialization succeeded.
    bool Initialize(std::string& outError);
    void Destroy(void);

    //Sets the basic parameters for a burst or update pass.
    void SetUpdatePassParams(UniformDictionary& params, bool isBurst,
                             Vector2f particleTexelMin, Vector2f particleTexelMax,
                             float timeStep = 0.0f,
                             MTexture2D* particleTexData1 = 0, MTexture2D* particleTexData2 = 0);
    //Sets up the basic uniforms used by the given render pass.
    void SetRenderPassParams(UniformDictionary& params,
                             float texelSize, Vector2f particleTexelMin,
                             MTexture2D* particleTexData1, MTexture2D* particleTexData2);

    void PuncherFire_Burst(Vector3f pos, Vector3f dir, Vector3f tangent, Vector3f bitangent);


private:

    ParticleContent(void) { }
};
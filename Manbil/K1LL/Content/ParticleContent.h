#pragma once

#include "../../Rendering/Rendering.hpp"



struct RenderPassVertex
{
    //Integer values indicating the particle's pixel coordinate offset in the texture data.
    Vector2f ID;
    RenderPassVertex(Vector2f id = Vector2f()) : ID(id) { }

    static const RenderIOAttributes VertexAttrs;
};


//A set of "burst", "update", and "render" materials.
struct ParticleMaterial
{
    static const std::string UNIFORM_TEX1, UNIFORM_TEX2;

    Material *BurstMat = 0,
             *UpdateMat = 0,
             *RenderMat = 0;
    UniformDictionary BurstParams, UpdateParams, RenderParams;
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


    void PuncherFire_Burst(Vector3f pos, Vector3f dir, Vector3f tangent, Vector3f bitangent);


private:

    ParticleContent(void) { }
};
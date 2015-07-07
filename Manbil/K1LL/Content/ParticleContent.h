#pragma once

#include "../../Rendering/Rendering.hpp"



//The vertices used for rendering particles.
//Just has an "ID" value corresponding to an integer.
struct ParticleVertex
{
    static RenderIOAttributes GetVertexAttributes(void)
    {
        return RenderIOAttributes(RenderIOAttributes::Attribute(2, false, "vIn_ID"));
    }

    Vector2f ID;

    ParticleVertex(Vector2f id = Vector2f()) : ID(id) { }
};


//A set of "burst", "update", and "render" materials.
struct ParticleMaterial
{
    Material *BurstMat = 0,
             *UpdateMat = 0,
             *RenderMat = 0;
    UniformDictionary BurstParams, UpdateParams, RenderParams;
};


class ParticleContent
{
public:

    static ParticleContent Instance;


    ParticleMaterial PuncherFire;


    //Returns whether initialization succeeded.
    bool Initialize(std::string& outError);
    void Destroy(void);


private:

    ParticleContent(void) { }
};
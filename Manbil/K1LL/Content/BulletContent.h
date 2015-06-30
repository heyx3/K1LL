#pragma once

#include "../../Rendering/Rendering.hpp"


class BulletContent
{
public:

    static BulletContent Instance;


    //Creates/loads all bullet content.
    //Returns true if everything was initialized correctly.
    //Outputs an error message and returns false if something failed.
    bool Initialize(std::string& outErrorMsg);

    //Destroys/unloads all the bullet content.
    //If "Initialize()" hasn't been called, nothing will happen.
    void Destroy(void);


    void RenderPuncherBullet(Vector3f pos, Vector3f dir, const RenderInfo& info);


private:

    Mesh bulletMesh;
    Material* bulletMat;
    UniformDictionary bulletParams;


    MTexture2D defaultTex;


    BulletContent(void);
};
#pragma once

#include "../../Rendering/Rendering.hpp"


class WeaponContent
{
public:

    static WeaponContent Instance;


    //Creates/loads all weapon content.
    //Returns true if everything was initialized correctly.
    //Outputs an error message and returns false if something failed.
    bool Initialize(std::string& outErrorMsg);

    //Destroys/unloads all the weapon content.
    //If "Initialize()" hasn't been called, nothing will happen.
    void Destroy(void);

    
    void RenderPuncher(Vector3f pos, Vector3f dir, const RenderInfo& info);
    void RenderLightGun(Vector3f pos, Vector3f dir, const RenderInfo& info);
    void RenderPRG(Vector3f pos, Vector3f dir, const RenderInfo& info);
    void RenderTerribleShotgun(Vector3f pos, Vector3f dir, const RenderInfo& info);
    void RenderSprayNPray(Vector3f pos, Vector3f dir, const RenderInfo& info);
    void RenderCluster(Vector3f pos, Vector3f dir, const RenderInfo& info);
    void RenderPOS(Vector3f pos, Vector3f dir, const RenderInfo& info);


private:

    //Submeshes are the various individual weapons.
    Mesh weaponMesh;

    Material* weaponMat;
    UniformDictionary weaponParams;

    MTexture2D texPuncher, texLightGun, texPRG, texPOS,
               texSprayNPray, texCluster, texTerribleShotgun;


    WeaponContent(void);


    void RenderWeapon(Vector3f pos, Vector3f dir, unsigned int meshIndex, float animSpeed,
                      MTexture2D* tex, const RenderInfo& info);
};
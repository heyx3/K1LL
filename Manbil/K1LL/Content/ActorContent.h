#pragma once

#include "../../Rendering/Rendering.hpp"
#include "../Level Info/ItemTypes.h"


class ActorContent
{
public:

    static ActorContent Instance;


    static const unsigned int NLightWeapons = 1,
                              NHeavyWeapons = 1,
                              NSpecialWeapons = 1;


    //The weapons being used for the current game. Determines which meshes get rendered.
    unsigned int LightWeaponIndex = 0,
                 HeavyWeaponIndex = 0,
                 SpecialWeaponIndex = 0;


    //Creates/loads all the actor content.
    //Returns true if everything was initialized correctly.
    //Outputs an error message and returns false if something failed.
    bool Initialize(std::string& outErrorMsg);

    //Destroys/unloads all the actor content.
    //If "Initialize()" hasn't been called, nothing will happen.
    void Destroy(void);


    //Gets the number of different available player meshes.
    unsigned int GetNPlayerMeshes(void) const { return playerMesh.SubMeshes.size(); }


    //Renders a player with the given information.
    void RenderPlayer(Vector2f pos, Vector3f forward, Vector3f color, unsigned int meshIndex,
                      const RenderInfo& info);
    //Renders a spawned item that the player can pick up.
    void RenderPickup(Vector2f pos, ItemTypes item, const RenderInfo& info);


private:

    //The player meshes. Each potential player mesh is a single MeshData instance.
    Mesh playerMesh;
    Material* playerMat;
    MTexture2D playerTex;
    UniformDictionary playerParams;

    Mesh lightAmmoMesh, heavyAmmoMesh, specialAmmoMesh;
    Material *lightAmmoMat, *heavyAmmoMat, *specialAmmoMat;
    UniformDictionary lightAmmoParams, heavyAmmoParams, specialAmmoParams;

    Mesh lightWeaponMesh, heavyWeaponMesh, specialWeaponMesh;
    Material *lightWeaponMat, *heavyWeaponMat, *specialWeaponMat;
    std::vector<UniformDictionary> lightWeaponParams, heavyWeaponParams, specialWeaponParams;
    
    Mesh healthMesh;
    Material* healthMat;
    UniformDictionary healthParams;

    
    ActorContent(void);
};
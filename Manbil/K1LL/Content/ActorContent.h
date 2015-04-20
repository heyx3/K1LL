#pragma once

#include "../../Rendering/Rendering.hpp"


class ActorContent
{
public:

    static ActorContent Instance;


    //Creates/loads all the actor content.
    //Returns true if everything was initialized correctly.
    //Outputs an error message and returns false if something failed.
    bool Initialize(std::string& outErrorMsg);

    //Destroys/unloads all the actor content.
    //If "Initialize()" hasn't been called, nothing will happen.
    void Destroy(void);


    //Gets the number of different available player meshes.
    unsigned int GetNPlayerMeshes(void) const { return playerMeshData.SubMeshes.size(); }


    //Renders a player with the given information.
    void RenderPlayer(Vector2f pos, Vector3f forward, Vector3f color, unsigned int meshIndex,
                      const RenderInfo& info);


private:

    //The player meshes. Each potential player mesh is a single MeshData instance.
    Mesh playerMeshData;
    Material* playerMeshMat;
    MTexture2D playerTex;
    UniformDictionary playerMeshParams;
    
    
    ActorContent(void);
};
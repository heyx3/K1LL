#include "ActorContent.h"

#include "../../Rendering/Data Nodes/DataNodes.hpp"
#include "../../Rendering/Data Nodes/ShaderGenerator.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace
{
    const std::string uniform_teamColor = "u_teamColor",
                      uniform_playerTex = "u_tex";
}


ActorContent ActorContent::Instance = ActorContent();

ActorContent::ActorContent(void)
    : playerTex(TextureSampleSettings2D(FT_LINEAR, WT_WRAP), PS_8U, true),
      playerMat(0), healthMat(0),
      lightAmmoMat(0), heavyAmmoMat(0), specialAmmoMat(0)
{

}


bool ActorContent::Initialize(std::string& err)
{
    Destroy();

    RenderIOAttributes playerVertices = VertexPosUVNormal::GetVertexAttributes();

    #pragma region Player meshes

    {
        std::vector<VertexPosUVNormal> vertices;
        std::vector<unsigned int> indices;

        Assimp::Importer importer;
        unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

        const unsigned int nMeshes = 5;
        for (unsigned int i = 0; i < nMeshes; ++i)
        {
            std::string file = "Content/Game/Meshes/Players/M" + std::to_string(i) + ".obj";
            const aiScene* scene = importer.ReadFile(file, flags);

            //Load the scene/mesh.
            if (scene == 0)
            {
                err = "Error loading player mesh '" + file + "': " + importer.GetErrorString();
                return false;
            }
            if (scene->mNumMeshes != 1)
            {
                err = "Mesh '" + file + "' should only have one mesh!";
                return false;
            }

            aiMesh* mesh = scene->mMeshes[0];


            //Make sure the mesh is valid.
            assert(mesh->HasFaces());
            if (!mesh->HasPositions() || !mesh->HasTextureCoords(0) || !mesh->HasNormals())
            {
                err = "Mesh '" + file + "' is missing positions, normals, or UVs!";
                return false;
            }


            //Populate the vertex/index buffer data.
            vertices.resize(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                vertices[i].Pos = *(Vector3f*)(&mesh->mVertices[i].x);
                vertices[i].UV = *(Vector2f*)(&mesh->mTextureCoords[0][i].x);
                vertices[i].Normal = *(Vector3f*)(&mesh->mNormals[i].x);
            }
            indices.resize(mesh->mNumFaces);
            for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
            {
                aiFace& fce = mesh->mFaces[i];
                if (fce.mNumIndices != 3)
                {
                    err = "A face in mesh '" + file + "' has a non-tri face with " +
                            std::to_string(fce.mNumIndices) + " indices!";
                    return false;
                }

                indices.push_back(fce.mIndices[0]);
                indices.push_back(fce.mIndices[1]);
                indices.push_back(fce.mIndices[2]);
            }

            //Create the vertex/index buffers.
            playerMesh.SubMeshes.push_back(MeshData(false, PT_TRIANGLE_LIST));
            MeshData& dat = playerMesh.SubMeshes[playerMesh.SubMeshes.size() - 1];
            dat.SetVertexData(vertices, MeshData::BUF_STATIC, playerVertices);
            dat.SetIndexData(indices, MeshData::BUF_STATIC);
        }
    }

    #pragma endregion

    #pragma region Player Texture

    {
        const std::string file = "Content/Game/Textures/Player.png";
        playerTex.Create();
        if (!playerTex.SetDataFromFile(file, err))
        {
            err = "Error loading '" + file + "': " + err;
            return false;
        }
    }

    #pragma endregion

    #pragma region Player Material

    {
        //Pretty straight-forward for now -- blend a texture with a color for the fragment output.
        //Will likely add more complex things in the future, so the world-space normal is also calculated.

        SerializedMaterial mat;
        mat.VertexInputs = playerVertices;

        DataLine vIn_Pos(VertexInputNode::GetInstance(), 0),
                 vIn_UV(VertexInputNode::GetInstance(), 1),
                 vIn_Normal(VertexInputNode::GetInstance(), 2);

        DataNode::Ptr screenPos = SpaceConverterNode::ObjPosToScreenPos(vIn_Pos, "screenPos");
        mat.MaterialOuts.VertexPosOutput = DataLine(screenPos, 1);
        
        DataNode::Ptr worldNormal(new SpaceConverterNode(vIn_Normal,
                                                         SpaceConverterNode::ST_OBJECT,
                                                         SpaceConverterNode::ST_WORLD,
                                                         SpaceConverterNode::DT_NORMAL,
                                                         "worldNormal"));
        mat.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_UV", vIn_UV));
        mat.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_WorldNormal", worldNormal));

        DataLine fIn_UV(FragmentInputNode::GetInstance(), 0);
        DataNode::Ptr texSampler(new TextureSample2DNode(fIn_UV, uniform_playerTex, "texSampler"));
        DataLine texRGB(texSampler, TextureSample2DNode::GetOutputIndex(CO_AllColorChannels));

        DataNode::Ptr blendColor(new ParamNode(3, uniform_teamColor));

        DataNode::Ptr finalColor(new CustomExpressionNode("vec4('0' * '1', 1.0)", 4,
                                                          texRGB, blendColor, "finalColor"));
        mat.MaterialOuts.FragmentOutputs.push_back(ShaderOutput("fOut_Color", finalColor));


        ShaderGenerator::GeneratedMaterial genM("");
        genM = ShaderGenerator::GenerateMaterial(mat, playerParams, BlendMode::GetOpaque());
        if (!genM.ErrorMessage.empty())
        {
            err = "Error generating player material: " + err;
            return false;
        }

        assert(playerTex.IsValidTexture());
        playerParams[uniform_playerTex].Tex() = playerTex.GetTextureHandle();
        playerParams[uniform_teamColor].Float().SetValue(Vector3f(1.0f, 1.0f, 1.0f));

        playerMat = genM.Mat;
    }

    #pragma endregion

    //TODO: ammo and health meshes/materials/uniforms. Light ammo has that pixellated color thing, heavy ammo rotates, special ammo has a bubbly world position offset thing.

    return true;
}
void ActorContent::Destroy(void)
{
    #define CLEAR_ALL(varNameBase) \
        if (varNameBase ## Mat != 0) { delete varNameBase ## Mat; varNameBase ## Mat = 0; } \
        varNameBase ## Mesh.SubMeshes.clear(); varNameBase ## Mesh.CurrentSubMesh = 0; \
        varNameBase ## Params.clear();


    CLEAR_ALL(player);
    playerTex.DeleteIfValid();
    
    CLEAR_ALL(lightAmmo);
    CLEAR_ALL(heavyAmmo);
    CLEAR_ALL(specialAmmo);
    CLEAR_ALL(health);

    
    #undef CLEAR_ALL
}

void ActorContent::RenderPlayer(Vector2f pos, Vector3f forward, Vector3f color, unsigned int meshIndex,
                                const RenderInfo& info)
{
    playerMesh.CurrentSubMesh = meshIndex;
    playerParams[uniform_teamColor].Float().SetValue(color);

    playerMesh.Transform.SetPosition(Vector3f(pos.x, pos.y, 0.0f));
    playerMesh.Transform.SetRotation(Quaternion(Vector3f(0.0f, 0.0f, 1.0f),
                                                atan2f(forward.y, forward.x)));

    playerMat->Render(info, &playerMesh, playerParams);
}
void ActorContent::RenderPickup(Vector2f pos, ItemTypes item, const RenderInfo& info)
{
    Material* mat;
    Mesh* mesh;
    UniformDictionary* params;
    float height;

    switch (item)
    {
        case IT_AMMO_LIGHT:
            mat = lightAmmoMat;
            mesh = &lightAmmoMesh;
            params = &lightAmmoParams;
            height = 2.0f;
            break;
        case IT_AMMO_HEAVY:
            mat = heavyAmmoMat;
            mesh = &heavyAmmoMesh;
            params = &heavyAmmoParams;
            height = 2.0f;
            break;
        case IT_AMMO_SPECIAL:
            mat = specialAmmoMat;
            mesh = &specialAmmoMesh;
            params = &specialAmmoParams;
            height = 2.0f;
            break;

        case IT_HEALTH:
            mat = healthMat;
            mesh = &healthMesh;
            params = &healthParams;
            height = 2.0f;
            break;

        default:
            assert(false);
            break;
    }

    mesh->Transform.SetPosition(Vector3f(pos, height));
    mat->Render(info, mesh, *params);
}
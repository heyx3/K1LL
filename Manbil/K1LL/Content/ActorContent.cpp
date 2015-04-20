#include "ActorContent.h"

#include "../../Rendering/Data Nodes/DataNodes.hpp"
#include "../../Rendering/Data Nodes/ShaderGenerator.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


const std::string uniform_teamColor = "u_teamColor",
                  uniform_playerTex = "u_tex";


ActorContent ActorContent::Instance = ActorContent();

ActorContent::ActorContent(void)
    : playerTex(TextureSampleSettings2D(FT_LINEAR, WT_WRAP), PS_8U, true),
      playerMeshMat(0)
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
            std::string file = "Content/Meshes/Players/M" + std::to_string(i) + ".obj";
            const aiScene* scene = importer.ReadFile(file, flags);

            //Load the scene/mesh.
            if (scene == 0)
            {
                err = "Error loading player mesh '" + file + "': " + importer.GetErrorString();
                return;
            }
            if (scene->mNumMeshes != 1)
            {
                err = "Mesh '" + file + "' should only have one mesh!";
                return;
            }

            aiMesh* mesh = scene->mMeshes[0];


            //Make sure the mesh is valid.
            assert(mesh->HasFaces());
            if (!mesh->HasPositions() || !mesh->HasTextureCoords(0) || !mesh->HasNormals())
            {
                err = "Mesh '" + file + "' is missing positions, normals, or UVs!";
                return;
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
                    return;
                }

                indices.push_back(fce.mIndices[0]);
                indices.push_back(fce.mIndices[1]);
                indices.push_back(fce.mIndices[2]);
            }

            //Create the vertex/index buffers.
            playerMeshData.SubMeshes.push_back(MeshData(false, PT_TRIANGLE_LIST));
            MeshData& dat = playerMeshData.SubMeshes[playerMeshData.SubMeshes.size() - 1];
            dat.SetVertexData(vertices, MeshData::BUF_STATIC, playerVertices);
            dat.SetIndexData(indices, MeshData::BUF_STATIC);
        }
    }

    #pragma endregion

    #pragma region Player Texture

    {
        const std::string file = "Content/Textures/Player.png";
        playerTex.Create();
        if (!playerTex.SetDataFromFile(file, err))
        {
            err = "Error loading '" + file + "': " + err;
            return;
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
        genM = ShaderGenerator::GenerateMaterial(mat, playerMeshParams, BlendMode::GetOpaque());
        if (!genM.ErrorMessage.empty())
        {
            err = "Error generating player material: " + err;
            return;
        }

        playerMeshMat = genM.Mat;

        assert(playerTex.IsValidTexture());
        playerMeshParams.Texture2Ds[uniform_playerTex].Texture = playerTex.GetTextureHandle();
    }

    #pragma endregion
}
void ActorContent::Destroy(void)
{
    if (playerMeshMat != 0)
    {
        delete playerMeshMat;
        playerMeshMat = 0;
    }
    playerMeshData.SubMeshes.clear();
    playerMeshData.CurrentSubMesh = 0;
    playerMeshParams.ClearUniforms();
    playerTex.DeleteIfValid();
}

void ActorContent::RenderPlayer(Vector2f pos, Vector3f forward, Vector3f color, unsigned int meshIndex,
                                const RenderInfo& info)
{
    playerMeshData.CurrentSubMesh = meshIndex;
    playerMeshParams.Floats[uniform_teamColor].SetValue(color);

    playerMeshData.Transform.SetPosition(Vector3f(pos.x, pos.y, 0.0f));
    playerMeshData.Transform.SetRotation(Quaternion(Vector3f(0.0f, 0.0f, 1.0f),
                                                    atan2f(forward.y, forward.x)));

    playerMeshMat->Render(info, &playerMeshData, playerMeshParams);
}
#include "BulletContent.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../Rendering/Data Nodes/DataNodes.hpp"
#include "../../Rendering/Data Nodes/ShaderGenerator.h"
#include "WeaponConstants.h"


namespace
{
    const std::string UNIFORM_TEXTURE = "u_tex",
                      UNIFORM_COLOR = "u_color";

    enum Bullets
    {
        B_PUNCHER = 0,
        B_TERRIBLE_SHOTGUN,
        B_SPRAY_N_PRAY,
        B_CLUSTER,

        B_NUMBER_OF_BULLETS
    };
}


BulletContent BulletContent::Instance = BulletContent();


BulletContent::BulletContent(void)
    : bulletMat(0),
      defaultTex(TextureSampleSettings2D(FT_NEAREST, WT_CLAMP), PS_8U, false)
{

}


bool BulletContent::Initialize(std::string& err)
{
    typedef VertexPosUV BulletVertex;
    RenderIOAttributes vertIns = BulletVertex::GetVertexAttributes();


    #pragma region Meshes

    for (unsigned int i = 0; i < B_NUMBER_OF_BULLETS; ++i)
    {
        bulletMesh.SubMeshes.push_back(MeshData(false, PT_TRIANGLE_LIST));
    }

    std::vector<BulletVertex> vertices;
    std::vector<unsigned int> indices;

    Assimp::Importer importer;
    unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

    for (unsigned int i = 0; i < B_NUMBER_OF_BULLETS; ++i)
    {
        //Get the file for this bullet type.
        std::string file = "Content/Game/Meshes/Bullets/";
        switch ((Bullets)i)
        {
            case B_PUNCHER:
                file += "Puncher.obj";
                break;
            case B_TERRIBLE_SHOTGUN:
                file += "Terrible Shotgun.obj";
                break;
            case B_SPRAY_N_PRAY:
                file += "Spray and Pray.obj";
                break;
            case B_CLUSTER:
                file += "Cluster.obj";
                break;

            default:
                assert(false);
        }

        const aiScene* scene = importer.ReadFile(file, flags);

        //Make sure the scene is valid.
        if (scene == 0)
        {
            err = "Error loading '" + file + "': " + importer.GetErrorString();
            return false;
        }
        if (scene->mNumMeshes != 1)
        {
            err = "Mesh '" + file + "' has " + std::to_string(scene->mNumMeshes) + " meshes in it";
            return false;
        }

        aiMesh* mesh = scene->mMeshes[0];

        //Make sure the mesh is valid.
        assert(mesh->HasFaces());
        if (!mesh->HasPositions() || !mesh->HasTextureCoords(0))
        {
            err = "Mesh '" + file + "' is missing positions or UVs!";
            return false;
        }

        //Populate the vertex/index buffer data.
        vertices.resize(mesh->mNumVertices);
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            vertices[j].Pos = *(Vector3f*)(&mesh->mVertices[j].x);
            vertices[j].UV = *(Vector2f*)(&mesh->mTextureCoords[0][j].x);
        }
        indices.resize(mesh->mNumFaces * 3);
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
        {
            aiFace& fce = mesh->mFaces[j];
            if (fce.mNumIndices != 3)
            {
                err = "A face in mesh '" + file + "' has a non-tri face with " +
                      std::to_string(fce.mNumIndices) + " indices!";
                return false;
            }

            indices[(j * 3)] = fce.mIndices[0];
            indices[(j * 3) + 1] = fce.mIndices[1];
            indices[(j * 3) + 2] = fce.mIndices[2];
        }

        //Create the vertex/index buffers.
        bulletMesh.SubMeshes[i].SetVertexData(vertices, MeshData::BUF_STATIC, vertIns);
        bulletMesh.SubMeshes[i].SetIndexData(indices, MeshData::BUF_STATIC);
    }

    #pragma endregion

    #pragma region Textures

    {
        Array2D<Vector4b> values(1, 1, Vector4b((unsigned char)255, 255, 255, 255));
        defaultTex.Create();
        defaultTex.SetColorData(values);
    }

    #pragma endregion

    #pragma region Materials

    {
        SerializedMaterial serMat;
        serMat.VertexInputs = vertIns;

        DataLine vIn_Pos(VertexInputNode::GetInstance(), 0),
                 vIn_UV(VertexInputNode::GetInstance(), 1);
        
        DataNode::Ptr screenPos = SpaceConverterNode::ObjPosToScreenPos(vIn_Pos, "screenPos");
        serMat.MaterialOuts.VertexPosOutput = DataLine(screenPos, 1);

        serMat.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_UV", vIn_UV));

        DataLine fIn_UV(FragmentInputNode::GetInstance(), 0);

        DataNode::Ptr tex(new TextureSample2DNode(fIn_UV, UNIFORM_TEXTURE, "texSample"));
        DataLine texRGB(tex, TextureSample2DNode::GetOutputIndex(CO_AllColorChannels));

        DataNode::Ptr colorParam(new ParamNode(3, UNIFORM_COLOR));

        DataNode::Ptr finalRGB(new MultiplyNode(texRGB, colorParam, "finalRGB"));
        DataNode::Ptr finalColor(new CombineVectorNode(finalRGB, 1.0f, "finalColor"));

        serMat.MaterialOuts.FragmentOutputs.push_back(ShaderOutput("fOut_Color", finalColor));

        auto genMat = ShaderGenerator::GenerateMaterial(serMat, bulletParams, BlendMode::GetOpaque());
        if (!genMat.ErrorMessage.empty())
        {
            err = "Error generating bullet material: " + genMat.ErrorMessage;
            return false;
        }
        bulletMat = genMat.Mat;
    }

    #pragma endregion

    return true;
}

void BulletContent::Destroy(void)
{
    bulletMesh.SubMeshes.clear();
    bulletMesh.CurrentSubMesh = 0;

    delete bulletMat;

    bulletParams.ClearUniforms();

    defaultTex.DeleteIfValid();
}

void BulletContent::RenderPuncherBullet(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    bulletMesh.Transform.SetPosition(pos);
    bulletMesh.Transform.SetRotation(Quaternion(WeaponConstants::Instance.WeaponForward, dir));

    bulletMesh.CurrentSubMesh = B_PUNCHER;

    bulletParams.Texture2Ds[UNIFORM_TEXTURE].Texture = defaultTex.GetTextureHandle();
    bulletParams.Floats[UNIFORM_COLOR].SetValue(WeaponConstants::Instance.PuncherMaterial.BulletColor);

    bulletMat->Render(info, &bulletMesh, bulletParams);
}
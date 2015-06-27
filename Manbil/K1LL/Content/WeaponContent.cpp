#include "WeaponContent.h"

#include "WeaponConstants.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace
{
    const std::string UNIFORM_TEXTURE = "u_tex",
                      UNIFORM_ANIMSPEED = "u_animSpeed";

    enum Weapons
    {
        W_PUNCHER = 0,
        W_LIGHTGUN,
        W_PRG,
        W_TERRIBLE_SHOTGUN,
        W_SPRAY_N_PRAY,
        W_CLUSTER,
        W_POS,

        W_NUMBER_OF_WEAPONS,
    };

    const Vector3f WEAPON_BASE_DIR(1.0f, 0.0f, 0.0f);
}


WeaponContent WeaponContent::Instance = WeaponContent();


#define WEAPON_TEX_ARGS TextureSampleSettings2D(FT_LINEAR, WT_WRAP), PS_8U, false

WeaponContent::WeaponContent(void)
    : weaponMat(0),
      texPuncher(WEAPON_TEX_ARGS), texPRG(WEAPON_TEX_ARGS), texPOS(WEAPON_TEX_ARGS),
      texLightGun(WEAPON_TEX_ARGS), texSprayNPray(WEAPON_TEX_ARGS),
      texCluster(WEAPON_TEX_ARGS), texTerribleShotgun(WEAPON_TEX_ARGS)
{

}

bool WeaponContent::Initialize(std::string& err)
{
    struct WeaponVertex
    {
        Vector3f Pos;
        Vector2f UV;
        Vector3f Normal, Tangent, Bitangent;
    };
    RenderIOAttributes weaponVertIns(RenderIOAttributes::Attribute(3, false, "vIn_Pos"),
                                     RenderIOAttributes::Attribute(2, false, "vIn_UV"),
                                     RenderIOAttributes::Attribute(3, true, "vIn_Normal"),
                                     RenderIOAttributes::Attribute(3, true, "vIn_Tangent"),
                                     RenderIOAttributes::Attribute(3, true, "vIn_Bitangent"));

    #pragma region Meshes

    {
        for (unsigned int i = 0; i < W_NUMBER_OF_WEAPONS; ++i)
        {
            weaponMesh.SubMeshes.push_back(MeshData(false, PT_TRIANGLE_LIST));
        }

        std::vector<WeaponVertex> vertices;
        std::vector<unsigned int> indices;

        Assimp::Importer importer;
        unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

        for (unsigned int i = 0; i < W_NUMBER_OF_WEAPONS; ++i)
        {
            //Get the file of this weapon.
            std::string file = "Content/Game/Meshes/Guns/";
            switch ((Weapons)i)
            {
                case W_PUNCHER:
                    file += "Puncher.obj";
                    break;
                case W_LIGHTGUN:
                    file += "Light Gun.obj";
                    break;
                case W_PRG:
                    file += "Perpetual Rail Gun.obj";
                    break;
                case W_TERRIBLE_SHOTGUN:
                    file += "Terrible Shotgun.obj";
                    break;
                case W_SPRAY_N_PRAY:
                    file += "Spray and Pray.obj";
                    break;
                case W_CLUSTER:
                    file += "Cluster.obj";
                    break;
                case W_POS:
                    file += "Personal Orbital Strike.obj";
                    break;

                default: assert(false);
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
                err = "Mesh '" + file + "' has " + std::to_string(scene->mNumMeshes) + "meshes in it";
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
            if (!mesh->HasTangentsAndBitangents())
            {
                err = "Mesh '" + file + "' is missing tangents/bitangents!";
                return false;
            }

            //Populate the vertex/index buffer data.
            vertices.resize(mesh->mNumVertices);
            for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
            {
                vertices[j].Pos = *(Vector3f*)(&mesh->mVertices[j].x);
                vertices[j].UV = *(Vector2f*)(&mesh->mTextureCoords[0][j].x);
                vertices[j].Normal = *(Vector3f*)(&mesh->mNormals[j].x);
                vertices[j].Tangent = *(Vector3f*)(&mesh->mTangents[j].x);
                vertices[j].Bitangent = *(Vector3f*)(&mesh->mBitangents[j].x);
            }
            indices.resize(mesh->mNumFaces);
            for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
            {
                aiFace& fce = mesh->mFaces[j];
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
            weaponMesh.SubMeshes[i].SetVertexData(vertices, MeshData::BUF_STATIC, weaponVertIns);
            weaponMesh.SubMeshes[i].SetIndexData(indices, MeshData::BUF_STATIC);
        }
    }

    #pragma endregion

    #pragma region Textures

    {
        #define MAKE_TEX(var, file) \
            var.Create(); \
            if (!var.SetDataFromFile(file, err)) \
            { \
                err = std::string("Error loading '") + file + "': " + err; \
                return false; \
            }
        MAKE_TEX(texPuncher, "Content/Game/Textures/Weapons/Puncher.png")
        MAKE_TEX(texLightGun, "Content/Game/Textures/Weapons/LightGun.png")
        MAKE_TEX(texPRG, "Content/Game/Textures/Weapons/PerpetualRailGun.png")
        MAKE_TEX(texPOS, "Content/Game/Textures/Weapons/POS.png")
        MAKE_TEX(texSprayNPray, "Content/Game/Textures/Weapons/SprayAndPray.png")
        MAKE_TEX(texCluster, "Content/Game/Textures/Weapons/Cluster.png")
        MAKE_TEX(texTerribleShotgun, "Content/Game/Textures/Weapons/Terrible Shotgun.png")
    }

    #pragma endregion

    #pragma region Material

    {
        //Vertex shader.
        std::string vOuts =
R"(\
out vec3 fIn_WorldPos, fIn_WorldNormal, fIn_WorldTangent, fIn_WorldBitangent;
out vec2 fIn_UV;\
)";
        MaterialUsageFlags vUse;
        vUse.EnableFlag(MaterialUsageFlags::DNF_USES_WVP_MAT);
        vUse.EnableFlag(MaterialUsageFlags::DNF_USES_WORLD_MAT);
        
        std::string u_wvp = MaterialConstants::WVPMatName,
                    u_world = MaterialConstants::WorldMatName;
        std::string vShader = MaterialConstants::GetVertexHeader(vOuts, weaponVertIns, vUse) +
R"(
void main()
{
    //Standard pass-through vertex shader.
    fIn_UV = vIn_UV;
    vec4 temp4 = )" + u_world + R"( * vec4(vIn_Pos, 1.0);
    fIn_WorldPos = temp4.xyz / temp4.w;
    fIn_WorldNormal = ( )" + u_world + R"( * vec4(vIn_Normal, 0.0)).xyz;
    fIn_WorldTangent = ( )" + u_world + R"( * vec4(vIn_Tangent, 0.0)).xyz;
    fIn_WorldBitangent = ( )" + u_world + R"( * vec4(vIn_Bitangent, 0.0)).xyz;

    gl_Position = )" + u_wvp + R"(vec4(vIn_Pos, 1.0);
})";

        //Fragment shader.
        std::string fIns =
R"(\
in vec3 fIn_WorldPos, fIn_WorldNormal, fIn_WorldTangent, fIn_WorldBitangent;
in vec2 fIn_UV;\
)";
        std::string fOuts = "out vec4 fOut_Color;";
        MaterialUsageFlags fUse;
        fUse.EnableFlag(MaterialUsageFlags::DNF_USES_TIME);

        std::string u_time = MaterialConstants::ElapsedTimeName;
        std::string fShader = MaterialConstants::GetFragmentHeader(fIns, fOuts, MaterialUsageFlags()) +
R"(
uniform sampler2D )" + UNIFORM_TEXTURE + R"(;
uniform float )" + UNIFORM_ANIMSPEED + R"(;

void main()
{
    //For each tangent vector, get a value from 0 to 1 indicating how close this fragment is
    //   to a grid line in that direction.

    const float gridSizeScale = 0.25;
    
    vec2 gridValsBase = vec2(dot(fIn_WorldPos, fIn_WorldTangent),
                             dot(fIn_WorldPos, fIn_WorldBitangent));
    gridValsBase += )" + UNIFORM_ANIMSPEED + " * " + u_time + R"(;
    vec2 gridVals = fract(gridSizeScale * gridValsBase);
    gridVals = pow(2.0 * abs(gridVals - vec2(0.5)), vec2(8.0));

    vec3 texRGB = texture2D( )" + UNIFORM_TEXTURE + R"(, fIn_UV).xyz;
    fOut_Color = vec4(texRGB * max(gridVals.x, gridVals.y), 1.0);
})";

        weaponMat = new Material(vShader, fShader, weaponParams, weaponVertIns,
                                 BlendMode::GetOpaque(), err);
        if (!err.empty())
        {
            err = "Error creating weapon material: " + err;
            return false;
        }
    }

    #pragma endregion


    return true;
}
void WeaponContent::Destroy(void)
{
    if (weaponMat != 0)
    {
        delete weaponMat;
    }

    texPuncher.DeleteIfValid();
    texLightGun.DeleteIfValid();
    texPRG.DeleteIfValid();
    texPOS.DeleteIfValid();
    texSprayNPray.DeleteIfValid();
    texCluster.DeleteIfValid();
    texTerribleShotgun.DeleteIfValid();

    weaponMesh.SubMeshes.clear();
    weaponMesh.CurrentSubMesh = 0;
    weaponParams.ClearUniforms();
}

void WeaponContent::RenderPuncher(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_PUNCHER, WeaponConstants::Instance.PuncherAnimSpeed,
                 &texPuncher, info);
}
void WeaponContent::RenderLightGun(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_LIGHTGUN, WeaponConstants::Instance.LightGunAnimSpeed,
                 &texLightGun, info);
}
void WeaponContent::RenderPRG(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_PRG, WeaponConstants::Instance.PRGAnimSpeed,
                 &texPRG, info);
}
void WeaponContent::RenderTerribleShotgun(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_TERRIBLE_SHOTGUN, WeaponConstants::Instance.TerribleShotgunAnimSpeed,
                 &texTerribleShotgun, info);
}
void WeaponContent::RenderSprayNPray(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_SPRAY_N_PRAY, WeaponConstants::Instance.SprayNPrayAnimSpeed,
                 &texSprayNPray, info);
}
void WeaponContent::RenderCluster(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_CLUSTER, WeaponConstants::Instance.ClusterAnimSpeed,
                 &texCluster, info);
}
void WeaponContent::RenderPOS(Vector3f pos, Vector3f dir, const RenderInfo& info)
{
    RenderWeapon(pos, dir, W_POS, WeaponConstants::Instance.POSAnimSpeed,
                 &texPOS, info);
}

void WeaponContent::RenderWeapon(Vector3f pos, Vector3f dir, unsigned int subMesh,
                                 float animSpeed, MTexture2D* tex, const RenderInfo& info)
{
    weaponMesh.CurrentSubMesh = subMesh;

    TransformObject& trans = weaponMesh.Transform;
    trans.SetPosition(pos);
    trans.SetRotation(Quaternion(WEAPON_BASE_DIR, dir));

    weaponParams.Texture2Ds[UNIFORM_TEXTURE].Texture = tex->GetTextureHandle();
    weaponParams.Floats[UNIFORM_ANIMSPEED].SetValue(animSpeed);

    weaponMat->Render(info, &weaponMesh, weaponParams);
}
#include "LevelGeometry.h"

#include "../../../Rendering/Primitives/PrimitiveGenerator.h"


LevelGeometry::LevelGeometry(const Array2D<BlockTypes>& levelGrid, std::string& err)
    : mat(0)
{
    typedef VertexPosNormal MyVert;
    RenderIOAttributes myVertAttrs = MyVert::GetVertexAttributes();


    #pragma region Generate vertices

    std::vector<MyVert> vertices;
    std::vector<unsigned int> indices;

    float ceilHeight = (float)Constants::Instance.CeilingHeight;

    //The six sides of the level bounds.
    std::vector<PrimitiveGenerator::CubemapVertex> vs;
    PrimitiveGenerator::GenerateCubemapCube(vs, indices, false, true,
                                            Vector3f(),
                                            Vector3f((float)levelGrid.GetWidth(),
                                                     (float)levelGrid.GetHeight(),
                                                     ceilHeight));
    for (unsigned int i = 0; i < vs.size(); ++i)
    {
        vertices.push_back(MyVert(vs[i].Pos, vs[i].Normal));
    }

    //Now build each exposed face of each wall.
    for (Vector2u counter; counter.y < levelGrid.GetHeight(); ++counter.y)
    {
        for (counter.x = 0; counter.x < levelGrid.GetWidth(); ++counter.x)
        {
            if (levelGrid[counter] == BT_WALL)
            {
#define ADD_WALL(v1, v2, v3, v4, normal) \
    vertices.push_back(MyVert(v1, normal)); \
    vertices.push_back(MyVert(v2, normal)); \
    vertices.push_back(MyVert(v3, normal)); \
    vertices.push_back(MyVert(v4, normal)); \
    indices.push_back(vertices.size() - 3); \
    indices.push_back(vertices.size() - 2); \
    indices.push_back(vertices.size() - 1); \
    indices.push_back(vertices.size() - 3); \
    indices.push_back(vertices.size() - 1); \
    indices.push_back(vertices.size() - 0);

                if (levelGrid[counter.LessX()] != BT_WALL)
                {
                    float minY = (float)counter.y,
                          maxY = minY + 1.0f;
                    float x = (float)counter.x;
                    ADD_WALL(Vector3f(x, minY, 0.0f), Vector3f(x, maxY, 0.0f),
                             Vector3f(x, maxY, ceilHeight), Vector3f(x, minY, ceilHeight),
                             Vector3f(-1.0f, 0.0f, 0.0f))
                }
                if (levelGrid[counter.LessY()] != BT_WALL)
                {
                    float minX = (float)counter.x,
                          maxX = minX + 1.0f;
                    float y = (float)counter.y;
                    ADD_WALL(Vector3f(minX, y, 0.0f), Vector3f(maxX, y, 0.0f),
                             Vector3f(maxX, y, ceilHeight), Vector3f(minX, y, ceilHeight),
                             Vector3f(0.0f, -1.0f, 0.0f))
                }
                if (levelGrid[counter.MoreX()] != BT_WALL)
                {
                    float minY = (float)counter.y,
                          maxY = minY + 1.0f;
                    float x = (float)(counter.x + 1);
                    ADD_WALL(Vector3f(x, minY, 0.0f), Vector3f(x, maxY, 0.0f),
                             Vector3f(x, maxY, ceilHeight), Vector3f(x, minY, ceilHeight),
                             Vector3f(1.0f, 0.0f, 0.0f))
                }
                if (levelGrid[counter.MoreY()] != BT_WALL)
                {
                    float minX = (float)counter.x,
                          maxX = minX + 1.0f;
                    float y = (float)(counter.y + 1);
                    ADD_WALL(Vector3f(minX, y, 0.0f), Vector3f(maxX, y, 0.0f),
                             Vector3f(maxX, y, ceilHeight), Vector3f(minX, y, ceilHeight),
                             Vector3f(0.0f, 1.0f, 0.0f))
                }
            }
        }
    }

    //Finally, generate the mesh data buffers.
    mesh.SubMeshes.push_back(MeshData(false, PrimitiveTypes::PT_TRIANGLE_LIST));
    mesh.SubMeshes[0].SetVertexData(vertices, MeshData::BUF_STATIC, myVertAttrs);
    mesh.SubMeshes[0].SetIndexData(indices, MeshData::BUF_STATIC);

    #pragma endregion


    #pragma region Generate material

    //TODO: Create material. Draw grids based on distance along up and tangent vectors.


    std::string vIn_Pos = myVertAttrs.GetAttribute(0).Name,
                vIn_Normal = myVertAttrs.GetAttribute(1).Name,
                u_vp = MaterialConstants::ViewProjMatName;
    std::string vOuts = "out vec3 fIn_WorldPos;\nout vec3 fIn_WorldNormal;";
    MaterialUsageFlags vUse;
    vUse.EnableFlag(MaterialUsageFlags::DNF_USES_VIEWPROJ_MAT);

    std::string vShader = MaterialConstants::GetVertexHeader(vOuts, myVertAttrs, vUse) +
"\nvoid main() \n\
{ \n\
    fIn_WorldPos = " + vIn_Pos + "; \n\
    fIn_WorldNormal = " + vIn_Normal + "; \n\
    gl_Position = " + u_vp + " * vec4(" + vIn_Pos + ", 1.0); \n\
}";

    std::string fIns = "in vec3 fIn_WorldPos;\nin vec3 fIn_WorldNormal;",
                fOuts = "out vec4 fOut_Color;";

    std::string fShader = MaterialConstants::GetFragmentHeader(fIns, fOuts, MaterialUsageFlags()) +
"\nvoid main() \n\
{ \n\
    const float gridSizeScale = 4.0; \n\
    vec3 tangent = cross(fIn_WorldNormal, vec3(0.0, 0.0, 1.0)); \n\
    \n\
    //For each vector (up and tangent), get a value indicating from 0 to 1 \n\
    //    how close this fragment is to a grid line in that direction. \n\
    vec2 gridVals = fract(gridSizeScale * vec2(fIn_WorldPos.z, dot(fIn_WorldPos, tangent))); \n\
    gridVals = pow(2.0 * abs(gridVals - 0.5), 2.5); \n\
    \n\
    fOut_Color = vec4(vec3(fIn_WorldPos.z / 16.0) + max(gridVals.x, gridVals.y), 1.0); \n\
}";

    mat = new Material(vShader, fShader, params, myVertAttrs, BlendMode::GetOpaque(), err);
    if (!err.empty())
    {
        err = "Error creating material: " + err;
        return;
    }

    //TODO: Make grids more closely-spaced as more players are nearby.
    //TODO: Add in pulse effects.


    #pragma endregion
}
LevelGeometry::~LevelGeometry(void)
{
    if (mat != 0)
    {
        delete mat;
    }
}

bool LevelGeometry::Update(Level* theLevel, float elapsedTime)
{
    return false;
}
void LevelGeometry::Render(Level* theLevel, float elapsedTime, const RenderInfo& info)
{
    mat->Render(info, &mesh, params);
}
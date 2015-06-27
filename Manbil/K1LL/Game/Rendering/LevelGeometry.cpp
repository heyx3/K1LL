#include "LevelGeometry.h"

#include "../../../Rendering/Primitives/PrimitiveGenerator.h"
#include "../Level/Level.h"


LevelGeometry::LevelGeometry(Level* theLevel, std::string& err)
    : mat(0), Actor(theLevel)
{
    const Array2D<BlockTypes>& levelGrid = theLevel->BlockGrid;

    typedef VertexPosNormal MyVert;
    RenderIOAttributes myVertAttrs = MyVert::GetVertexAttributes();


    #pragma region Generate vertices

    std::vector<MyVert> vertices;
    std::vector<unsigned int> indices;

    float ceilHeight = LevelConstants::Instance.CeilingHeight;

    //For every empty space, build the walls that border it.

#define ADD_WALL(v1, v2, v3, v4, normal) \
    vertices.push_back(MyVert(v1, normal)); \
    vertices.push_back(MyVert(v2, normal)); \
    vertices.push_back(MyVert(v3, normal)); \
    vertices.push_back(MyVert(v4, normal)); \
    indices.push_back(vertices.size() - 4); \
    indices.push_back(vertices.size() - 3); \
    indices.push_back(vertices.size() - 2); \
    indices.push_back(vertices.size() - 4); \
    indices.push_back(vertices.size() - 2); \
    indices.push_back(vertices.size() - 1);

    Vector2f sizeF = ToV2f(levelGrid.GetDimensions());

    for (Vector2u counter; counter.y < levelGrid.GetHeight(); ++counter.y)
    {
        for (counter.x = 0; counter.x < levelGrid.GetWidth(); ++counter.x)
        {
            assert(levelGrid[counter] != BT_DOORWAY);

            if (levelGrid[counter] == BT_NONE || levelGrid[counter] == BT_SPAWN)
            {
                if (counter.x == 0 || levelGrid[counter.LessX()] == BT_WALL)
                {
                    float minY = (float)counter.y,
                          maxY = minY + 1.0f;
                    float x = (float)counter.x;
                    ADD_WALL(Vector3f(x, minY, 0.0f), Vector3f(x, maxY, 0.0f),
                             Vector3f(x, maxY, ceilHeight), Vector3f(x, minY, ceilHeight),
                             Vector3f(1.0f, 0.0f, 0.0f))
                }
                if (counter.y == 0 || levelGrid[counter.LessY()] == BT_WALL)
                {
                    float minX = (float)counter.x,
                          maxX = minX + 1.0f;
                    float y = (float)counter.y;
                    ADD_WALL(Vector3f(minX, y, 0.0f), Vector3f(maxX, y, 0.0f),
                             Vector3f(maxX, y, ceilHeight), Vector3f(minX, y, ceilHeight),
                             Vector3f(0.0f, 1.0f, 0.0f))
                }
                if (counter.x == levelGrid.GetWidth() - 1 || levelGrid[counter.MoreX()] == BT_WALL)
                {
                    float minY = (float)counter.y,
                          maxY = minY + 1.0f;
                    float x = (float)(counter.x + 1);
                    ADD_WALL(Vector3f(x, minY, 0.0f), Vector3f(x, maxY, 0.0f),
                             Vector3f(x, maxY, ceilHeight), Vector3f(x, minY, ceilHeight),
                             Vector3f(1.0f, 0.0f, 0.0f))
                }
                if (counter.y == levelGrid.GetHeight() - 1 || levelGrid[counter.MoreY()] == BT_WALL)
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

    //Add the floors/ceiling.
    ADD_WALL(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, sizeF.y, 0.0f),
             Vector3f(sizeF.x, sizeF.y, 0.0f), Vector3f(sizeF.x, 0.0f, 0.0f),
             Vector3f(0.0f, 0.0f, 1.0f));
    ADD_WALL(Vector3f(0.0f, 0.0f, ceilHeight), Vector3f(0.0f, sizeF.y, ceilHeight),
             Vector3f(sizeF.x, sizeF.y, ceilHeight), Vector3f(sizeF.x, 0.0f, ceilHeight),
             Vector3f(0.0f, 0.0f, -1.0f));

    //Finally, generate the mesh data buffers.
    mesh.SubMeshes.push_back(MeshData(false, PrimitiveTypes::PT_TRIANGLE_LIST));
    mesh.SubMeshes[0].SetVertexData(vertices, MeshData::BUF_STATIC, myVertAttrs);
    mesh.SubMeshes[0].SetIndexData(indices, MeshData::BUF_STATIC);

    #pragma endregion


    #pragma region Generate material


    std::string vIn_Pos = myVertAttrs.GetAttribute(0).Name,
                vIn_Normal = myVertAttrs.GetAttribute(1).Name,
                u_vp = MaterialConstants::ViewProjMatName;
    std::string vOuts = "out vec3 fIn_WorldPos;\nout vec3 fIn_WorldNormal;";
    MaterialUsageFlags vUse;
    vUse.EnableFlag(MaterialUsageFlags::DNF_USES_VIEWPROJ_MAT);

    std::string vShader = MaterialConstants::GetVertexHeader(vOuts, myVertAttrs, vUse) +
R"(
void main()
{
    //Standard vertex shader for vertices that are in world space already.
    fIn_WorldPos = )" + vIn_Pos + R"(;
    fIn_WorldNormal = )" + vIn_Normal + R"(;
    gl_Position = )" + u_vp + " * vec4(" + vIn_Pos + R"( , 1.0);
})";

    std::string fIns = "in vec3 fIn_WorldPos;\nin vec3 fIn_WorldNormal;",
                fOuts = "out vec4 fOut_Color;";

    std::string fShader = MaterialConstants::GetFragmentHeader(fIns, fOuts, MaterialUsageFlags()) +
R"(
void main()
{
    //For each vector (up and tangent), get a value indicating from 0 to 1
    //    how close this fragment is to a grid line in that direction.

    const float gridSizeScale = 4.0;
    vec3 tangent = cross(fIn_WorldNormal, vec3(0.0, 0.0, 1.0));
   
    vec2 gridVals = fract(gridSizeScale * vec2(fIn_WorldPos.z, dot(fIn_WorldPos, tangent)));
    gridVals = pow(2.0 * abs(gridVals - vec2(0.5)), vec2(5.0));
    
    fOut_Color = vec4(vec3((fIn_WorldPos.z / 16.0) + max(gridVals.x, gridVals.y)), 1.0);
})";

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

bool LevelGeometry::Update(float elapsedTime)
{
    //Never destroy this actor.
    return false;
}
void LevelGeometry::Render(float elapsedTime, const RenderInfo& info)
{
    mat->Render(info, &mesh, params);
}
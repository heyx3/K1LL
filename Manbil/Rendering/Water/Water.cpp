#include "Water.h"

#include "../../OpenGLIncludes.h"
#include "../../RenderDataHandler.h"
#include "../../Math/Higher Math/Terrain.h"
#include "../../Material.h"



void CreateWaterMesh(unsigned int size, Vector3f scle, Mesh& outM)
{
    Vector3f offset(size * -0.5f, size * -0.5f, 0.0f);

    Array2D<float> terrainHeight(size, size, 0.0f);

    //Just create a flat terrain and let it do the math.
    Terrain terr(Vector2u(size, size));
    terr.SetHeightmap(terrainHeight);
    std::vector<WaterVertex> verts;
    std::vector<unsigned int> indices;
    terr.GenerateTrianglesFull<WaterVertex>(verts, indices,
                                            [](WaterVertex& v) { return &v.Pos; },
                                            [](WaterVertex& v) { return &v.TexCoord; });

    //Convert the terrain vertices into water vertices.
    for (unsigned int i = 0; i < verts.size(); ++i)
        verts[i].Pos = verts[i].Pos.ComponentProduct(scle) + offset;


    //Create the vertex and index buffers.
    RenderObjHandle vbo, ibo;
    RenderDataHandler::CreateVertexBuffer(vbo, verts.data(), verts.size(),
                                          RenderDataHandler::BufferPurpose::UPDATE_ONCE_AND_DRAW);
    RenderDataHandler::CreateIndexBuffer(ibo, indices.data(), indices.size(),
                                         RenderDataHandler::BufferPurpose::UPDATE_ONCE_AND_DRAW);
    Vector2u terrSize(terr.GetWidth(), terr.GetHeight());
    Vector2u nVerts = Terrain::GetNVertices(terrSize);
    VertexIndexData vid(nVerts.x * nVerts.y, vbo,
                        Terrain::GetNIndices(terrSize), ibo);
    outM.SubMeshes.insert(outM.SubMeshes.end(), vid);
}

Water::Water(unsigned int size, Vector3f pos, Vector3f scale,
             OptionalValue<RippleWaterCreationArgs> rippleArgs,
             OptionalValue<DirectionalWaterCreationArgs> directionArgs)
    : currentRippleIndex(0), totalRipples(0), nextRippleID(0),
      currentFlowIndex(0), totalFlows(0), nextFlowID(0),
      rippleIDs(0), dp_tsc_h_p(0), sXY_sp(0),
      flowIDs(0), f_a_p(0), tsc(0),
      waterMesh(PrimitiveTypes::TriangleList)
{
    //Create mesh.
    CreateWaterMesh(size, scale, waterMesh);
    waterMesh.Transform.SetPosition(pos);


    //Set up ripples.
    if (rippleArgs.HasValue())
    {
        RippleWaterCreationArgs ripArgs = rippleArgs.GetValue();
        maxRipples = ripArgs.MaxRipples;

        rippleIDs = new unsigned int[maxRipples];
        dp_tsc_h_p = new Vector4f[maxRipples];
        sXY_sp = new Vector3f[maxRipples];
        for (unsigned int i = 0; i < maxRipples; ++i)
        {
            dp_tsc_h_p[i].w = 999.0f;
            dp_tsc_h_p[i].x = 0.001f;
            sXY_sp[i].z = 0.001f;
        }
        Params.FloatArrayUniforms["dropoffPoints_timesSinceCreated_heights_periods"] = UniformArrayValueF(&(dp_tsc_h_p[0][0]), maxRipples, 4, "dropoffPoints_timesSinceCreated_heights_periods");
        Params.FloatArrayUniforms["sourcesXY_speeds"] = UniformArrayValueF(&(sXY_sp[0][0]), maxRipples, 3, "sourcesXY_speeds");
    }
    else
    {
        maxRipples = 0;
    }

    //Set up flows.
    if (directionArgs.HasValue())
    {
        DirectionalWaterCreationArgs dirArgs = directionArgs.GetValue();
        maxFlows = dirArgs.MaxFlows;

        flowIDs = new unsigned int[maxFlows];
        f_a_p = new Vector4f[maxFlows];
        tsc = new float[maxFlows];

        for (unsigned int i = 0; i < maxFlows; ++i)
        {
            f_a_p[i] = Vector4f(0.001f, 0.0f, 0.0f, 9999.0f);
            tsc[i] = 0.0f;
        }
        Params.FloatArrayUniforms["flow_amplitude_period"] = UniformArrayValueF(&f_a_p[0][0], maxFlows, 4, "flows_amplitudes_periods");
        Params.FloatArrayUniforms["timesSinceCreated"] = UniformArrayValueF(tsc, maxFlows, 1, "timesSinceCreated");
    }
    else
    {
        maxFlows = 0;
    }
}
Water::~Water(void)
{
    if (rippleIDs != 0)
    {
        delete[] rippleIDs, dp_tsc_h_p, sXY_sp;
    }
    if (flowIDs != 0)
    {
        delete[] flowIDs, f_a_p, tsc;
    }
}

unsigned int Water::AddRipple(const RippleWaterArgs & args)
{
    assert(maxRipples > 0);

    RippleWaterArgs cpy(args);

    //Translate the source into object space.
    Matrix4f inv;
    waterMesh.Transform.GetWorldTransform(inv);
    inv = inv.GetInverse();
    cpy.Source = inv.Apply(args.Source);
    

    //Update tracking values.
    unsigned int rippleID = nextRippleID;
    nextRippleID += 1;
    unsigned int index = currentRippleIndex;
    currentRippleIndex += 1;
    currentRippleIndex %= maxRipples;

    //Make sure uniform values aren't invalid.
    if (args.DropoffPoint <= 0.0f)
        cpy.DropoffPoint = 9999.0f;
    if (args.Period <= 0.0f)
        cpy.Period = 5.0f;
    if (args.Speed <= 0.0f)
        cpy.Speed = 1.0f;

    //Set the uniforms.
    dp_tsc_h_p[index] = Vector4f(cpy.DropoffPoint, cpy.TimeSinceCreated, cpy.Amplitude, cpy.Period);
    sXY_sp[index] = Vector3f(cpy.Source.x, cpy.Source.y, cpy.Speed);
    rippleIDs[index] = rippleID;


    return rippleID;
}
bool Water::ChangeRipple(unsigned int element, const RippleWaterArgs & args)
{
    assert(maxRipples > 0);

    for (unsigned int i = 0; i < maxRipples; ++i)
    {
        if (rippleIDs[i] == element)
        {
            //Translate the source into object space.
            Matrix4f inv;
            waterMesh.Transform.GetWorldTransform(inv);
            inv = inv.GetInverse();
            Vector3f sourcePos = args.Source;
            sourcePos = inv.Apply(sourcePos);

            //Set the uniforms.
            dp_tsc_h_p[i] = Vector4f(args.DropoffPoint, args.TimeSinceCreated, args.Amplitude, args.Period);
            sXY_sp[i] = Vector3f(sourcePos.x, sourcePos.y, args.Speed);

            return true;
        }
    }

    return false;
}

unsigned int Water::AddFlow(const DirectionalWaterArgs & args)
{
    assert(maxFlows > 0);

    //Create a copy of the arguments so that any invalid uniform values can be changed.
    DirectionalWaterArgs cpy(args);
    if (cpy.Flow == Vector2f())
        cpy.Flow = Vector2f(0.001f, 0.0f);
    if (cpy.Period <= 0.0f)
        cpy.Period = 0.001f;

    //Update tracking values.
    unsigned int flowID = nextFlowID;
    nextFlowID += 1;
    unsigned int index = currentFlowIndex;
    currentFlowIndex += 1;
    currentFlowIndex %= maxFlows;

    //Set the uniforms.
    f_a_p[index] = Vector4f(cpy.Flow.x, cpy.Flow.y, cpy.Amplitude, cpy.Period);
    tsc[index] = cpy.TimeSinceCreated;


    return flowID;
}
bool Water::ChangeFlow(unsigned int element, const DirectionalWaterArgs & args)
{
    assert(maxFlows > 0);

    for (unsigned int i = 0; i < maxFlows; ++i)
    {
        if (flowIDs[i] == element)
        {
            //Set the uniforms.
            f_a_p[i] = Vector4f(args.Flow.x, args.Flow.y, args.Amplitude, args.Period);
            tsc[i] = args.TimeSinceCreated;

            return true;
        }
    }

    return false;
}

void Water::UpdateUniformLocations(const Material * mat)
{
    std::vector<UniformList::Uniform> fArrUs = mat->GetUniforms().FloatArrayUniforms;
    Params.FloatArrayUniforms["dropoffPoints_timesSinceCreated_heights_periods"].Location =
        UniformList::FindUniform("dropoffPoints_timesSinceCreated_heights_periods", fArrUs).Loc;
    Params.FloatArrayUniforms["sourcesXY_speeds"].Location =
        UniformList::FindUniform("sourcesXY_speeds", fArrUs).Loc;
    Params.FloatArrayUniforms["flow_amplitude_period"].Location =
        UniformList::FindUniform("flow_amplitude_period", fArrUs).Loc;
    Params.FloatArrayUniforms["timesSinceCreated"].Location =
        UniformList::FindUniform("timesSinceCreated", fArrUs).Loc;
}

void Water::Update(float elapsed)
{
    if (maxRipples > 0)
    {
        //Keep in mind that negative time values indicate the ripple source was stopped and is fading out.
        for (unsigned int i = 0; i < maxRipples; ++i)
            if (dp_tsc_h_p[i].y >= 0.0f)
                dp_tsc_h_p[i].y += elapsed;
            else dp_tsc_h_p[i].y -= elapsed;

    }

    if (maxFlows > 0)
    {
        for (unsigned int i = 0; i < maxFlows; ++i)
        {
            if (tsc[i] > 0.0f)
                tsc[i] += elapsed;
            else tsc[i] -= elapsed;
        }
    }

    UpdateMeshUniforms();
}

void Water::UpdateMeshUniforms(void)
{
    if (maxRipples > 0)
    {
        Params.FloatArrayUniforms["dropoffPoints_timesSinceCreated_heights_periods"].SetData(&(dp_tsc_h_p[0][0]));
        Params.FloatArrayUniforms["sourcesXY_speeds"].SetData(&(sXY_sp[0][0]));
    }
    if (maxFlows > 0)
    {
        Params.FloatArrayUniforms["flow_amplitude_period"].SetData(&f_a_p[0][0]);
        Params.FloatArrayUniforms["timesSinceCreated"].SetData(tsc);
    }
}
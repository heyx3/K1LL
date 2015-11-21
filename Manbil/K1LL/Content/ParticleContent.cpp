#include "ParticleContent.h"

#include "../../Rendering/Primitives/DrawingQuad.h"
#include "../Game/Rendering/ParticleManager.h"


namespace
{
    //"tex 1" and "tex 2" are the particle texture data. They are floating-point textures.
    //"pixel min" is the UV coordinate of the min corner of the sub-rectangle
    //    for the particles currently being simulated.
    //"n pixels inv" is 1.0 / the number of pixels along one axis in the full texture data.
    //"time step" is the elapsed time this update frame, in seconds.
    const std::string UNIFORM_TEXEL_SIZE = "u_texelSize",
                      UNIFORM_PARTICLE_TEXEL_MIN = "u_particleTexelMin",
                      UNIFORM_PARTICLE_TEXEL_MAX = "u_particleTexelMax",
                      UNIFORM_TIME_STEP = "u_timeStep",
                      UNIFORM_TEX1 = "u_tex1",
                      UNIFORM_TEX2 = "u_tex2";


    //Update/burst passes.
    
    typedef VertexPosUV UpdatePassVertex;

    //Generates the vertex shader for the update/burst passes.
    std::string GetVertexShader_Update(void)
    {
        RenderIOAttributes vertIns = UpdatePassVertex::GetVertexAttributes();
        return MaterialConstants::GetVertexHeader("out vec2 fIn_UV;\n", vertIns,
                                                  MaterialUsageFlags()) +
R"(
uniform vec2 )" + UNIFORM_PARTICLE_TEXEL_MIN + ", " + UNIFORM_PARTICLE_TEXEL_MAX + R"(;

void main()
{
    fIn_UV = mix( )" + UNIFORM_PARTICLE_TEXEL_MIN + R"( ,
                  )" + UNIFORM_PARTICLE_TEXEL_MAX + R"( ,
                  )" + vertIns.GetAttribute(1).Name + R"( );
    gl_Position = vec4( )" + vertIns.GetAttribute(0).Name + R"( , 1.0);
}
)";
    }
    //Each invocation of this shader runs on a single particle.
    //The "shaderBody" parameter should provide any custom declarations and the main() funtion.
    //The main() function can use the following variables:
    //   -UNIFORM_TIME_STEP: the amount of time elapsed this frame (if not a "burst" material).
    //   -MaterialConstants::ElapsedTimeName: the total elapsed time since the particles were emitted.
    //   -"fIn_UV": the UV coordinates of the particle for the data textures.
    //   -UNIFORM_TEX1: the first particle data texture (if not a "burst" material).
    //   -UNIFORM_TEX2: the second particle data texture (if not a "burst" material).
    //The following outputs should be written to:
    //   -"fOut_Tex1": the XYZW values for the first particle data texture.
    //   -"fOut_Tex2": the XYZW values for the second particle data texture.
    std::string GetFragmentShader_Update(bool isBurstMaterial, const std::string& shaderBody)
    {
        return MaterialConstants::GetFragmentHeader("in vec2 fIn_UV;\n", "out vec4 fOut_Tex1, fOut_Tex2;\n",
                                                    MaterialUsageFlags(MaterialUsageFlags::DNF_USES_TIME)) +
               (isBurstMaterial ?
                    "" :
                    ("uniform sampler2D " + UNIFORM_TEX1 + ", " +
                                            UNIFORM_TEX2 + ";\n" +
                     "uniform float " + UNIFORM_TIME_STEP + ";\n\n")) +
               "\n\n" + shaderBody;
    }

    //Sets up the material and parameter dictionary for an update/burst pass with the given fragment shader.
    void GenerateUpdatePass(const std::string& fragmentShader, bool isBurst,
                            UniformDictionary& params, Material** outM, std::string& outErrorMsg)
    {
        params[UNIFORM_PARTICLE_TEXEL_MIN] = Uniform(UNIFORM_PARTICLE_TEXEL_MIN, UT_VALUE_F);
        params[UNIFORM_PARTICLE_TEXEL_MIN].Float().SetValue(Vector2f());
        params[UNIFORM_PARTICLE_TEXEL_MAX] = Uniform(UNIFORM_PARTICLE_TEXEL_MAX, UT_VALUE_F);
        params[UNIFORM_PARTICLE_TEXEL_MAX].Float().SetValue(Vector2f());
        if (!isBurst)
        {
            params[UNIFORM_TIME_STEP] = Uniform(UNIFORM_TIME_STEP, UT_VALUE_F);
            params[UNIFORM_TIME_STEP].Float() = 0.0f;   
            params[UNIFORM_TEX1] = Uniform(UNIFORM_TEX1, UT_VALUE_SAMPLER2D);
            params[UNIFORM_TEX2] = Uniform(UNIFORM_TEX2, UT_VALUE_SAMPLER2D);
        }

        *outM = new Material(GetVertexShader_Update(), fragmentShader, params,
                             DrawingQuad::GetVertexInputData(), BlendMode::GetOpaque(),
                             outErrorMsg);
    }


    //Render pass.

    //Creates the vertex shader for the "render" pass.
    //The parameters should provide code for the following things, respectively:
    //     -Declarations of any custom outputs -- if you wish to sample from extra particle data textures,
    //        do it in this shader and output it into the next one (geometry shader).
    //        The two particle data textures already get output, so don't worry about them.
    //     -Declarations of anything else -- uniforms, helper functions, etc.
    //     -Code for the body of the main() function that sets the custom outputs declared above,
    //        as well as the world-space position output "gIn_WorldPos".
    //The following fields are available inside the main function:
    //     -"uv": The UV coordinates of this particle in the particle data textures.
    //     -"gIn_Tex1": The RGBA value of the first texture.
    //     -"gIn_Tex2": The RGBA value of the second texture.
    //     -"vIn_ID": A vec2 containing the pixel offset of this particle along each axis in the texture.
    //     -UNIFORM_TEXEL_SIZE: The size of a single texel in the particle texture data.
    //     -UNIFORM_PARTICLE_TEXEL_MIN: The min corner of the sub-rectangle in the particle texture data
    //        being rendered.
    //     -UNIFORM_TEX1: The first data texture.
    //     -UNIFORM_TEX2: The second data texture.
    std::string GetVertexShader_Render(const std::string& customOutputs,
                                       const std::string& declarations,
                                       const std::string& setOutputs)
    {
        return MaterialConstants::GetVertexHeader("out vec4 gIn_Tex1, gIn_Tex2;\nout vec3 gIn_WorldPos;\n" +
                                                    customOutputs + "\n",
                                                  RenderPassVertex::VertexAttrs,
                                                  MaterialUsageFlags()) +
               declarations +
R"(
uniform float )" + UNIFORM_TEXEL_SIZE + R"(;
uniform vec2 )" + UNIFORM_PARTICLE_TEXEL_MIN + R"(;
uniform sampler2D )" + UNIFORM_TEX1 + ", " + UNIFORM_TEX2 + R"(;

void main()
{
    vec2 uv = )" + UNIFORM_PARTICLE_TEXEL_MIN +
                   " + (" + RenderPassVertex::VertexAttrs.GetAttribute(0).Name +
                        " * " + UNIFORM_TEXEL_SIZE + R"( );
    gIn_Tex1 = texture2D( )" + UNIFORM_TEX1 + R"( , uv);
    gIn_Tex2 = texture2D( )" + UNIFORM_TEX2 + R"( , uv);

    //TODO: See if this line is needed.
    gl_Position = vec4(1.0);

    )" + setOutputs +
"\n}";
    }
    //Creates the geometry shader for the "render" pass, which expands each point into a billboard.
    //The parameters should provide code for the following things, respectively:
    //     -Declarations of any custom inputs from the vertex shader. Since this is a geometry shader,
    //        all inputs must have "[1]" on the end of them -- one value for each input vertex.
    //     -Declarations of any custom outputs -- if you passed in extra outputs from the vertex shader
    //        and want them to pass-through to the fragment shader, declare those fragment shader inputs here.
    //     -Declarations of anything else -- uniforms, helper functions, etc.
    //     -Code to be run multiple times, setting your custom outputs for each vertex in the billboard.
    //     -Optional code that rotates/scales the vec3 variables "up" and "side",
    //        which rotates/scales the billboard that gets created.
    //The following fields are available for the code's body:
    //     -MaterialConstants::ViewProjMatName
    //     -MaterialConstants::CamForwardName
    //     -MaterialConstants::CamSidewaysName
    //     -MaterialConstants::CamUpwardsName
    //     -"fIn_Tex1": The first particle data texture value.
    //     -"fIn_Tex2": The second particle data texture value.
    //     -"gIn_WorldPos[0]": The world position from the vertex shader.
    //     -"side": A tangent vector for the billboard's surface.
    //     -"up": A second tangent vector for the billboard's surface.
    //     -Whatever custom inputs you declared.
    //The following functions are available for the code's body:
    //     -"vec4 makeQuat(vec3 axis, float angle)": Makes a quaternion for the given rotation.
    //     -"vec3 applyQuat(vec4 quat, vec3 point)": Applies a quaternion's rotation to a point.
    //   Note that these are intended to be used to rotate the "up" and "side" vectors
    //     along the camera's forward axis.
    std::string GetGeometryShader_Render(const std::string& customInputs,
                                         const std::string& customOutputs,
                                         const std::string& declarations,
                                         const std::string& setOutputs,
                                         const std::string modifyUpAndSide = "")
    {
        MaterialUsageFlags usage(MaterialUsageFlags::DNF_USES_VIEWPROJ_MAT,
                                 MaterialUsageFlags::DNF_USES_CAM_FORWARD,
                                 MaterialUsageFlags::DNF_USES_CAM_SIDEWAYS,
                                 MaterialUsageFlags::DNF_USES_CAM_UPWARDS);
        return MaterialConstants::GetGeometryHeader("in vec4 gIn_Tex1[1], gIn_Tex2[1];\nin vec3 gIn_WorldPos[1]" +
                                                      customInputs + "\n\n" +
                                                     "out vec4 fIn_Tex1, fIn_Tex2;\n" +
                                                      customOutputs + "\n\n",
                                                    PT_POINTS, PT_TRIANGLE_STRIP, 4, usage) +
               declarations + R"(
vec4 makeQuat(vec3 axis, float angle)
{
    float halfRot = 0.5 * angle;
    return vec4(axis * sin(halfRot), cos(halfRot));
}
vec3 applyQuat(vec4 quat, vec3 point)
{
    return point + (2.0 * cross(cross(point, quat.xyz) + (pos * quat.w),
                                quat.xyz));
}

void main()
{
    fIn_Tex1 = gIn_Tex1[0];
    fIn_Tex2 = gIn_Tex2[0];

    vec3 side = )" + MaterialConstants::CameraSideName + R"(,
         up = )" + MaterialConstants::CameraUpName + ";\n" +
    modifyUpAndSide + R"(
    vec3 plusSide = gIn_WorldPos[0] + side,
         minusSide = gIn_WorldPos[0] - side;

    gl_Position = )" + MaterialConstants::ViewProjMatName + R"( * vec4(plusSide + up, 1.0);
    fIn_Tex1 = gIn_Tex1[0];
    fIn_Tex2 = gIn_Tex2[0];
    )" + setOutputs + R"(
    EmitVertex();

    gl_Position = )" + MaterialConstants::ViewProjMatName + R"( * vec4(plusSide - up, 1.0);
    fIn_Tex1 = gIn_Tex1[0];
    fIn_Tex2 = gIn_Tex2[0];
    )" + setOutputs + R"(
    EmitVertex();

    gl_Position = )" + MaterialConstants::ViewProjMatName + R"( * vec4(minusSide + up, 1.0);
    fIn_Tex1 = gIn_Tex1[0];
    fIn_Tex2 = gIn_Tex2[0];
    )" + setOutputs + R"(
    EmitVertex();

    gl_Position = )" + MaterialConstants::ViewProjMatName + R"( * vec4(minusSide - up, 1.0);
    fIn_Tex1 = gIn_Tex1[0];
    fIn_Tex2 = gIn_Tex2[0];
    )" + setOutputs + R"(
    EmitVertex();
}
)";
    }
    //Creates the fragment shader for the "render" pass, which just outputs the particle's color.
    //The parameters should provide code for the following things, respectively:
    //     -Declarations of any custom inputs from the geometry shader.
    //     -The main() function, which should just set the RGBA output "fOut_Color".
    //The following fields are available for the code's body:
    //     -"fIn_Tex1": The first particle data texture value.
    //     -"fIn_Tex2": The second particle data texture value.
    //     -Whatever custom inputs you declared.
    std::string GetFragmentShader_Render(const std::string& customInputs, const std::string& mainFunction)
    {
        return MaterialConstants::GetFragmentHeader("in vec4 fIn_Tex1, fIn_Tex2;\n" + customInputs,
                                                    "out vec4 fOut_Color;\n", MaterialUsageFlags()) + "\n\n" +
               mainFunction;
    }

    //Sets up the material and parameter dictionary for a render pass with the given shaders.
    void GenerateRenderPass(const std::string& vertexShader,
                            const std::string& geoShader,
                            const std::string& fragmentShader,
                            UniformDictionary& params, Material** outM, std::string& outErrorMsg)
    {
        params[UNIFORM_TEXEL_SIZE] = Uniform(UNIFORM_TEXEL_SIZE, UT_VALUE_F);
        params[UNIFORM_TEXEL_SIZE].Float().SetValue(0.0f);
        params[UNIFORM_PARTICLE_TEXEL_MIN] = Uniform(UNIFORM_PARTICLE_TEXEL_MIN, UT_VALUE_F);
        params[UNIFORM_PARTICLE_TEXEL_MIN].Float().SetValue(Vector2f());
        params[UNIFORM_TEX1] = Uniform(UNIFORM_TEX1, UT_VALUE_SAMPLER2D);
        params[UNIFORM_TEX2] = Uniform(UNIFORM_TEX2, UT_VALUE_SAMPLER2D);

        *outM = new Material(GetVertexShader_Update(), fragmentShader, params,
                             DrawingQuad::GetVertexInputData(), BlendMode::GetOpaque(),
                             outErrorMsg);
    }



    void DestroyMat(ParticleMaterial& mat)
    {
        if (mat.BurstMat != 0)
        {
            delete mat.BurstMat;
            mat.BurstMat = 0;
        }
        if (mat.UpdateMat != 0)
        {
            delete mat.UpdateMat;
            mat.UpdateMat = 0;
        }
        if (mat.RenderMat != 0)
        {
            delete mat.RenderMat;
            mat.RenderMat = 0;
        }

        mat.BurstParams.clear();
        mat.UpdateParams.clear();
        mat.RenderParams.clear();
    }
}

const RenderIOAttributes RenderPassVertex::VertexAttrs =
        RenderIOAttributes(RenderIOAttributes::Attribute(2, false, "vIn_ID"));


ParticleContent ParticleContent::Instance = ParticleContent();


bool ParticleContent::Initialize(std::string& outError)
{
    RenderIOAttributes updateVertIns = DrawingQuad::GetVertexInputData();
    const std::string update_pos = updateVertIns.GetAttribute(0).Name,
                      update_id = updateVertIns.GetAttribute(0).Name;

    const RenderIOAttributes& renderVertIns = RenderPassVertex::VertexAttrs;
    const std::string vertInID = renderVertIns.GetAttribute(0).Name;

    #pragma region Puncher bullet fire

    //Texture1.rgb is the position.
    //Texture1.a is the ?.
    //Texture2.rgb is the velocity.
    //Texture2.a is the ?.
    {
        std::string vShader, gShader, fShader;

        //Burst:
        fShader = GetFragmentShader_Update(true, R"(
uniform vec3 u_shootDir, u_shootTangent, u_shootBitangent, u_shootOrigin;
void main()
{
    vec3 pos = u_shootOrigin +
               (mix(-u_shootTangent, u_shootTangent, fIn_UV.x) * 0.15) +
               (mix(-u_shootBitangent, u_shootBitangent, fIn_UV.y) * 0.15) +
               u_shootDir * 0.15;
    fOut_Tex1 = vec4(pos, 0.0);
    fOut_Tex2 = vec4((pos - u_shootOrigin) * 1.0, 0.0);
}
)");
        PuncherFire.BurstParams["u_shootDir"] = Uniform("u_shootDir", UT_VALUE_F);
        PuncherFire.BurstParams["u_shootDir"].Float().SetValue(Vector3f());
        PuncherFire.BurstParams["u_shootTangent"] = Uniform("u_shootTangent", UT_VALUE_F);
        PuncherFire.BurstParams["u_shootTangent"].Float().SetValue(Vector3f());
        PuncherFire.BurstParams["u_shootBitangent"] = Uniform("u_shootBitangent", UT_VALUE_F);
        PuncherFire.BurstParams["u_shootBitangent"].Float().SetValue(Vector3f());
        PuncherFire.BurstParams["u_shootOrigin"] = Uniform("u_shootOrigin", UT_VALUE_F);
        PuncherFire.BurstParams["u_shootOrigin"].Float().SetValue(Vector3f());
        GenerateUpdatePass(fShader, true, PuncherFire.BurstParams, &PuncherFire.BurstMat, outError);
        if (!outError.empty())
        {
            outError = "Error creating burst for puncher fire: " + outError;
            return false;
        }


        //Update:
        fShader = GetFragmentShader_Update(false, R"(
void main()
{
    vec4 tex1 = texture2D( )" + UNIFORM_TEX1 + R"(, fIn_UV),
         tex2 = texture2D( )" + UNIFORM_TEX2 + R"(, fIn_UV);

    fOut_Tex1 = vec4(tex1.rgb + (tex2.rgb * )" + UNIFORM_TIME_STEP + R"(), tex1.a);
    fOut_Tex2 = vec4(tex2.rgb * 0.999, tex2.a);
})");
        GenerateUpdatePass(fShader, false, PuncherFire.UpdateParams, &PuncherFire.UpdateMat, outError);
        if (!outError.empty())
        {
            outError = "Error creating update for puncher fire: " + outError;
            return false;
        }


        //Render:
        vShader = GetVertexShader_Render("", "", "gIn_WorldPos = gIn_Tex1.rgb;");
        gShader = GetGeometryShader_Render("", "", "", "", "up *= 0.01; side *= 0.01;");
        fShader = GetFragmentShader_Render("", "void main() { fOut_Color = vec4(vec3(0.5), 1.0); }");
        GenerateRenderPass(vShader, gShader, fShader,
                           PuncherFire.RenderParams, &PuncherFire.RenderMat,
                           outError);
        if (!outError.empty())
        {
            outError = "Error creating render for puncher fire: " + outError;
            return false;
        }
    }

    #pragma endregion


    return true;
}
void ParticleContent::Destroy(void)
{
    DestroyMat(PuncherFire);
}


void ParticleContent::SetUpdatePassParams(UniformDictionary& params, bool isBurst,
                                          Vector2f particleTexelMin, Vector2f particleTexelMax,
                                          float timeStep,
                                          MTexture2D* particleTexData1, MTexture2D* particleTexData2)
{
    params[UNIFORM_PARTICLE_TEXEL_MIN].Float().SetValue(particleTexelMin);
    params[UNIFORM_PARTICLE_TEXEL_MAX].Float().SetValue(particleTexelMax);
    if (!isBurst)
    {
        assert(particleTexData1 != 0 && particleTexData2 != 0);
        params[UNIFORM_TEX1].Tex() = particleTexData1->GetTextureHandle();
        params[UNIFORM_TEX2].Tex() = particleTexData2->GetTextureHandle();

        params[UNIFORM_TIME_STEP].Float().SetValue(timeStep);
    }
}
void ParticleContent::SetRenderPassParams(UniformDictionary& params,
                                          float texelSize, Vector2f particleTexelMin,
                                          MTexture2D* particleTexData1, MTexture2D* particleTexData2)
{
    params[UNIFORM_TEXEL_SIZE].Float().SetValue(texelSize);
    params[UNIFORM_PARTICLE_TEXEL_MIN].Float().SetValue(particleTexelMin);
    params[UNIFORM_TEX1].Tex() = particleTexData1->GetTextureHandle();
    params[UNIFORM_TEX2].Tex() = particleTexData2->GetTextureHandle();
}

void ParticleContent::PuncherFire_Burst(Vector3f pos, Vector3f dir, Vector3f tangent, Vector3f bitangent)
{
    PuncherFire.BurstParams["u_shootDir"].Float().SetValue(dir);
    PuncherFire.BurstParams["u_shootTangent"].Float().SetValue(tangent);
    PuncherFire.BurstParams["u_shootBitangent"].Float().SetValue(bitangent);
    PuncherFire.BurstParams["u_shootOrigin"].Float().SetValue(pos);

    ParticleManager::GetInstance().Burst(ParticleManager::S_MEDIUM, &PuncherFire, 3.0f);
}
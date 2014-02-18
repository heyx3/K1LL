#include "SFML/Graphics.hpp"

//#include "NoiseTest.h"
//#include "OpenGLTestWorld.h"
//#include "TwoTrianglesWorld.h"
//#include "Tests/TwoDOpenGLTest.h"


#include <iostream>
using namespace std;
void PauseConsole(void)
{
    char dummy;
    cout << "Enter any character to continue.\n";
    cin >> dummy;
}
void WriteToConsole(const std::string & outStr)
{
    cout << outStr << "\n";
}




#include "Rendering/Materials/Data Nodes/DataNodeIncludes.h"
#include "Rendering/Materials/Data Nodes/ShaderGenerator.h"
void TestDataNodes(void)
{
    #pragma region Set up channels

    std::unordered_map<RenderingChannels, DataLine> channels;

    //Diffuse = vec3(10, -32.4, 24) + vec3(-10, 32.4, -24)
    channels[RenderingChannels::RC_Diffuse] = DataLine(DataNodePtr(new AddNode(DataLine(Vector(10.0f, -32.4f, 24.0f)),
                                                                               DataLine(Vector(-10.0f, 32.4f, -24.0f)))),
                                                       0);
    //Object-space offset = texture2D([sampler], In_UV + (vec2(0.0) + (vec2(1.0) * (vec2(0.0) * u_elapsed_seconds))))
    channels[RenderingChannels::RC_ObjectVertexOffset] =
        DataLine(DataNodePtr(new TextureSampleNode(ChannelsOut::CO_AllColorChannels,
                                                   DataLine(DataNodePtr(new AddNode(DataLine(Vector(1.0f, 0.0f)),
                                                                                    DataLine(DataNodePtr(new ParamNode(2, "uvOffset")), 0))), 0))),
                             0);

    //World-space offset = vec3(40, 1, -12) / vec3(1, 1, 4) / pow(1.0, 5.0)
    std::vector<DataLine> toDivide;
    toDivide.insert(toDivide.begin(), DataLine(Vector(1.0f, 1.0f, 4.0f)));
    toDivide.insert(toDivide.end(), DataLine(DataNodePtr(new PowNode(DataLine(Vector(1.0f)), DataLine(Vector(5.0f)))), 0));
    channels[RenderingChannels::RC_WorldVertexOffset] = DataLine(DataNodePtr(new DivideNode(DataLine(Vector(40.0f, 1.0f, -12.0f)), toDivide)), 0);

    //Diffuse intensity = -In_Color.x
    DataNodePtr splitVec(new VectorComponentsNode(DataLine(DataNodePtr(new ObjectColorNode()), 0)));
    std::vector<DataLine> colorIn;
    colorIn.insert(colorIn.end(), DataLine(splitVec, 0));
    channels[RenderingChannels::RC_DiffuseIntensity] = DataLine(DataNodePtr(new NegativeNode(DataLine(DataNodePtr(new CombineVectorNode(colorIn)), 0))), 0);

    #pragma endregion


    UniformDictionary uniforms;

    std::string vert, frag;

    LightSettings lightSettings(false);
    ShaderGenerator::GenerateShaders(vert, frag, uniforms, RenderingModes::RM_Opaque, false, lightSettings, channels);

    WriteToConsole("Vertex shader:\n----------------------------------------------\n\n");
    WriteToConsole(vert);
    WriteToConsole("------------------------------------\n\nFragment shader:\n----------------------------------------\n\n");
    WriteToConsole(frag);

    PauseConsole();
}







int main()
{
    TestDataNodes();

    //NoiseTest().RunWorld();
    //OpenGLTestWorld().RunWorld();
    //TwoDOpenGLTest().RunWorld();
    //TwoTrianglesWorld().RunWorld();
}
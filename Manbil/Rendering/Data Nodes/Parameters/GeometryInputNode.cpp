#include "GeometryInputNode.h"


ADD_NODE_REFLECTION_DATA_CPP_SINGLETON(GeometryInputNode)


std::shared_ptr<DataNode> GeometryInputNode::instance = std::shared_ptr<DataNode>(new GeometryInputNode());


unsigned int GeometryInputNode::GetNumbOutputs(void) const
{
    AssertGeoShader();
    return GetMatData()->MaterialOuts.VertexOutputs.size();
}

unsigned int GeometryInputNode::GetOutputSize(unsigned int outputIndex) const
{
    return GetMatData()->MaterialOuts.VertexOutputs[outputIndex].Value.GetSize();
}
std::string GeometryInputNode::GetOutputName(unsigned int outputIndex) const
{
    return GetMatData()->MaterialOuts.VertexOutputs[outputIndex].Name;
}


void GeometryInputNode::AssertGeoShader(void) const
{
    Assert(CurrentShader == SH_GEOMETRY,
           "Invalid shader type (should be Geometry): " + ToString(CurrentShader));
}


#pragma warning(disable: 4100)
void GeometryInputNode::WriteMyOutputs(std::string & outCode) const
{
    AssertGeoShader();
    //Don't actually output anything, since the output name is an "in" variable.
}
#pragma warning(default: 4100)


GeometryInputNode::GeometryInputNode(void) : DataNode(std::vector<DataLine>(), GetInstanceName()) { }
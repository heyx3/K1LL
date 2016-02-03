#pragma once

#include "../DataNode.h"
#include "../../Textures/TextureChannels.h"


//Outputs the result of sampling a 2D texture.
class TextureSample2DNode : public DataNode
{
public:

    //Gets the output index for a 2D texture sample node that outputs the given data.
    static unsigned int GetOutputIndex(ChannelsOut channel);


    std::string SamplerName;

    TextureSample2DNode(const DataLine & uvs, std::string _samplerName = "", std::string name = "");


    virtual unsigned int GetNumbOutputs(void) const override { return 6; }

    virtual unsigned int GetOutputSize(unsigned int index) const override;
    virtual std::string GetOutputName(unsigned int index) const override;


protected:

    virtual void GetMyParameterDeclarations(UniformList& uniforms) const override;
    virtual void WriteMyOutputs(std::string & outCode) const override;
    
    virtual void WriteExtraData(DataWriter* writer) const override;
    virtual void ReadExtraData(DataReader* reader) override;

    virtual void AssertMyInputsValid(void) const override;

    virtual std::string GetInputDescription(unsigned int index) const override;


private:

    std::string GetSampleOutputName(void) const { return GetName() + "_sampled"; }

    ADD_NODE_REFLECTION_DATA_H(TextureSample2DNode)
};
#pragma once

#include "../Parameters/CameraDataNode.h"


//Computes surface light using ambient, diffuse, and specular components.
//Outputs a float representing the surface brightness.
class LightingNode : public DataNode
{
public:

    virtual unsigned int GetOutputSize(unsigned int index) const override;
    virtual std::string GetOutputName(unsigned int outputIndex) const override;

    LightingNode(const DataLine & surfaceWorldPos, const DataLine & surfaceWorldNormal, const DataLine & lightDir,
                 DataLine ambient = DataLine(0.2f), DataLine diffuse = DataLine(0.8f),
                 DataLine specular = DataLine(1.5f), DataLine specIntensity = DataLine(128.0f),
                 DataLine camPos = CameraDataNode::GetCamPos(), std::string name = "");


protected:

    virtual void GetMyFunctionDeclarations(std::vector<std::string> & outDecls) const override;
    virtual void WriteMyOutputs(std::string & outCode) const override;

    virtual std::string GetInputDescription(unsigned int index) const override;

    virtual void AssertMyInputsValid(void) const override;


private:

    std::string GetFuncName(void) const { return GetName() + "_getBrightness"; }

    const DataLine & GetSurfacePosInput() const { return GetInputs()[0]; }
    const DataLine & GetSurfaceNormalInput() const { return GetInputs()[1]; }
    const DataLine & GetLightDirInput() const { return GetInputs()[2]; }
    const DataLine & GetAmbientInput() const { return GetInputs()[3]; }
    const DataLine & GetDiffuseInput() const { return GetInputs()[4]; }
    const DataLine & GetSpecularInput() const { return GetInputs()[5]; }
    const DataLine & GetSpecularIntensityInput() const { return GetInputs()[6]; }
    const DataLine & GetCameraPosInput() const { return GetInputs()[7]; }

    static std::vector<DataLine> MakeInputVector(const DataLine & amb, const DataLine & diff, const DataLine & spec, const DataLine & specIntense,
                                                 const DataLine & camPos, const DataLine & surfPos,
                                                 const DataLine & surfNormal, const DataLine & lightDir);

    ADD_NODE_REFLECTION_DATA_H(LightingNode)
};
#pragma once

#include "../DataNode.h"


//Rotates a vector around an axis.
//TODO: If the rotation axis and angle is constant, precompute the quaternion when generating the GLSL code.
class RotateAroundAxisNode : public DataNode
{
public:

    virtual std::string GetName(void) const override { return "rotateAroundAxisNode"; }

    RotateAroundAxisNode(const DataLine & toRotate, const DataLine & rotateAxis, const DataLine & rotateAmount)
        : DataNode(MakeVector(toRotate, rotateAxis, rotateAmount), MakeVector(3))
    {
        assert(toRotate.GetDataLineSize() == 3 &&
               rotateAxis.GetDataLineSize() == 3 &&
               rotateAmount.GetDataLineSize() == 1);
    }

protected:

    virtual void GetMyFunctionDeclarations(std::vector<std::string> & outDecls) const override
    {
        //TODO: Each instance of this node has its own function that takes in the axis and angle. However, if the axis or angle is a constant value, DON'T take it in -- just use it.

        std::string getFN = GetGetQuatRotFuncName(),
                    applyFN = GetApplyQuatRotFuncName();

        //Only define the functions if they haven't been made already.
        if (std::any_of(outDecls.begin(), outDecls.end(), [getFN, applyFN](const std::string & str)
                {
                    return str.find(getFN) != std::string::npos ||
                           str.find(applyFN) != std::string::npos;
                }))
            return;


        std::string getFunc = "vec4 ";
        getFunc += getFN + "(vec3 axis, float rotateAmount)\n\
                           {\n\
                                float sinHalfAngle = sin(rotateAmount * 0.5),\n\
                                      cosHalfAngle = cos(rotateAmount * 0.5);\n\
                                return vec4(axis.xyz * sinHalfAngle, cosHalfAngle);\n\
                           }\n";
        outDecls.insert(outDecls.end(), getFunc);

        getFunc = "vec3 ";
        getFunc += applyFN + "(vec3 toRotate, vec4 quaternion)\n\
                              {\n\
                                  return toRotate + (2.0 * cross(cross(vector, quaternion.xyz) +\n\
                                                                   (quaternion.w * toRotate),\n\
                                                                 quaternion.xyz));\n\
                              }\n\n";
        outDecls.insert(outDecls.end(), getFunc);
    }
    virtual void WriteMyOutputs(std::string & outCode) const override
    {
        outCode += "\tvec3 " + GetOutputName(0) + " = " +
                        GetApplyQuatRotFuncName() + "(" + GetToRotateInput().GetValue() + ", " +
                                                          GetGetQuatRotFuncName() + "(" + GetRotateAxisInput().GetValue() + ", " +
                                                                                          GetRotateAmountInput().GetValue() + "));\n";
    }

private:

    static std::string GetGetQuatRotFuncName(void) { return "rotateAroundAxis_GetQuatRot"; }
    static std::string GetApplyQuatRotFuncName(void) { return "rotateAroundAxis_ApplyQuatRot"; }

    const DataLine & GetToRotateInput(void) const { return GetInputs()[0]; }
    const DataLine & GetRotateAxisInput(void) const { return GetInputs()[1]; }
    const DataLine & GetRotateAmountInput(void) const { return GetInputs()[2]; }
};
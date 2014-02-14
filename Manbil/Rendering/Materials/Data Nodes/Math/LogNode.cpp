#include "LogNode.h"

void LogNode::WriteMyOutputs(std::string & outCode) const
{
    std::string vecType = Vector(GetInputs()[0].GetDataLineSize()).GetGLSLType();


    if (GetInputs()[1].IsConstant && GetInputs()[1].GetConstantValue().GetValue()[0] == 2.0f)
    {
        outCode += "\t" + vecType + " " + GetOutputName(0) +
                    " = log2(" + GetInputs()[0].GetValue() + ");\n";
    }
    else
    {
        outCode += "\t" + vecType + " " + GetOutputName(0) +
                        " = log2(" + GetInputs()[0].GetValue() + ") /\n\t\t" +
                            "log2(" + GetInputs()[1].GetValue() + ");\n";
    }
}
#pragma once

#include <vector>
#include <assert.h>
#include <memory>
#include "../../../Math/Vectors.h"
#include "DataLine.h"




//Represents a basic, atomic operation in a shader.
class DataNode
{
public:

    typedef std::shared_ptr<DataNode> DataNodePtr;

    //Gets the identifier unique for this dataNode.
    unsigned int GetUniqueID(void) const { return id; }

    //Gets the name of this data node. MUST BE A VALID GLSL VARIABLE/FUNCTION NAME.
    virtual std::string GetName(void) const { return "unknownNode"; }


    DataNode(const std::vector<DataLine> & _inputs, const std::vector<unsigned int> & outputSizes)
        : id(GetNextID()), inputs(_inputs), outputs(outputSizes) { }
    DataNode(const DataNode & cpy); // Intentionally left blank.

    //The bit of code that will actually generate the glsl code.
    //Appends the code to the end of "outCode". Takes in a list of all nodes that have already added their code to "outCode".
    void WriteOutputs(std::string & outCode, std::vector<unsigned int> & writtenNodeIDs) const;

    //Gets any uniforms this node defines.
    virtual void GetParameterDeclarations(std::vector<std::string> & outDecls) { }
    //Gets any GLSL helper function declarations this node needs to use.
    virtual void GetFunctionDeclarations(std::vector<std::string> & outDecls) { }


    //Gets this node's input lines.
    const std::vector<DataLine> & GetInputs(void) const { return inputs; }
    //Gets this node's input lines.
    std::vector<DataLine> & GetInputs(void) { return inputs; }
    //Gets this node's output lines. Each element represents the size of that output line's Vector.
    const std::vector<unsigned int> & GetOutputs(void) const { return outputs; }
    //Gets this node's output lines. Each element represents the size of that output line's Vector.
    std::vector<unsigned int> & GetOutputs(void) { return outputs; }

    //Gets the variable name for this node's given output.
    //TODO: Allow subclasses to specify custom names for each output index.
    std::string GetOutputName(unsigned int outputIndex) const { return GetName() + std::to_string(id) + "_" + std::to_string(outputIndex); }


protected:

    static std::vector<DataLine> MakeVector(const DataLine & dat);
    static std::vector<DataLine> MakeVector(const DataLine & dat, const DataLine & dat2);
    static std::vector<DataLine> MakeVector(const DataLine & dat, const DataLine & dat2, const DataLine & dat3);
    static std::vector<DataLine> MakeVector(const DataLine & dat, const DataLine & dat2, const DataLine & dat3, const DataLine & dat4);
    static std::vector<DataLine> MakeVector(const DataLine & dat, std::vector<DataLine>::const_iterator wherePut, const std::vector<DataLine> & moreDats);

    static std::vector<unsigned int> MakeVector(unsigned int dat);
    static std::vector<unsigned int> MakeVector(unsigned int dat, unsigned int dat2);
    static std::vector<unsigned int> MakeVector(unsigned int dat, unsigned int dat2, unsigned int dat3);
    static std::vector<unsigned int> MakeVector(unsigned int dat, unsigned int dat2, unsigned int dat3, unsigned int dat4);


    //Writes the output for this node, assuming all inputs have already written their output.
    virtual void WriteMyOutputs(std::string & outCode) const = 0;


private:

    std::vector<DataLine> inputs;
    std::vector<unsigned int> outputs;


    static unsigned int nextID;
    static unsigned int GetNextID(void) { unsigned int id = nextID; nextID += 1; return id; }

    unsigned int id;
};
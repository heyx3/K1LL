#pragma once


#include "../../IO/DataSerialization.h"



//The type of simple data that can be used in place of a DAGNode.
//Using "void" for this value results in a DATInput with no default value type.
template<typename DefaultValueType>
//Represents an input to a DAG node.
//May be either a DAGNode output or some kind of default value type.
class DAGInput : public ISerializable
{
public:


    DAGInput(std::string nodeName = "", unsigned int nodeOutIndex = 0)
        : nonConstantName(nodeName), nonConstantOutIndex(dagNodeOutIndex) { }
    DAGInput(const DefaultValueType & constantVal)
        : constantValue(constantVal), nonConstantName(""), nonConstantOutIndex(0) { }

    virtual ~DAGInput(void) { }


    //Gets whether this input is a constant value (as opposed to a DAG node output).
    bool IsConstant(void) const { return nonConstantName.empty(); }
    //Gets whether this input is constant, and equal to the given value.
    bool IsConstant(const DefaultValueType & toCompare) const { return IsConstant() && constantValue == toCompare; }


    //Gets the value of this input, assuming it's constant.
    const DefaultValueType & GetConstantValue(void) const { assert(IsConstant()); return constantValue; }
    //Gets the DAG node this input points to, assuming this input isn't constant.
    const std::string & GetDAGNode(void) const { assert(!IsConstant()); return nonConstantName; }
    //Gets the DAG node's output index, assuming this input isn't constant.
    unsigned int GetDAGNodeOutIndex(void) const { assert(!IsConstant()); return nonConstantOutIndex; }


    //Sets the value of this input to the given constant value.
    //If it was already pointing to a DAG node output, that will be overwritten.
    void SetConstantInput(const DefaultValueType & defVal) { nonConstantName = ""; constantValue = defVal; }
    //Sets the value of this input to the given DAG node.
    //If it was already pointing to a constant value, that will be overwritten.
    void SetDAGNodeInput(const std::string & nodeName, unsigned int nodeOutputIndex)
    {
        nonConstantName = nodeName;
        nonConstantOutIndex = nodeOutputIndex;
    }


    bool ReadData(DataReader * data, std::string & outError) override
    {
        if (IsConstant())
        {
            return ReadConstantData(data, outError);
        }
        else
        {
            MaybeValue<std::string> tryStr = data->ReadString(outError);
            if (!tryStr.HasValue())
            {
                outError = "Error reading the DAG node name: " + outError;
                return false;
            }
            nonConstantNode = tryStr.GetValue();

            MaybeValue<unsigned int> tryUInt = data->ReadUInt(outError);
            if (!tryUInt.HasValue())
            {
                outError = "Error reading the DAG node out index: " + outError;
                return false;
            }
            nonConstantOutIndex = tryUInt.GetValue();

            return true;
        }
    }
    bool WriteData(DataWriter * data, std::string & outError) const override
    {
        if (IsConstant())
        {
            return WriteConstantValue(data, outError);
        }
        else
        {
            if (!data->WriteString(nonConstantNode, "Node Name", outError))
            {
                outError = "Error writing out the node's name, '" + nonConstantNode + "': " + outError;
                return false;
            }
            if (!data->WriteUInt(nonConstantOutIndex, "Node output index", outError))
            {
                outError = "Error writing out the node's output index " +
                               std::to_string(nonConstantOutIndex) + ": " + outError;
                return false;
            }
            return true;
        }
    }


protected:

    virtual bool ReadConstantValue(DataReader * data, std::string & outError) = 0;
    virtual bool WriteConstantValue(DataWriter * data, std::string & outError) = 0;


private:

    std::string nonConstantNode;
    unsigned int nonConstantOutIndex;

    DefaultValueType constantValue;
};



//Template specialization for inputs with no default value type.

template<>
class DAGInput<void> : public ISerializable
{
public:


    DAGInput(std::string _nodeName = "", unsigned int _nodeOutIndex = 0)
        : nodeName(_nodeName), nodeOutIndex(_nodeOutIndex) { }

    virtual ~DAGInput(void) { }


    //Returns false (this type of DAGInput doesn't have a default constant value type).
    bool IsConstant(void) const { return false; }


    //Gets the DAG node this input points to.
    const std::string & GetDAGNode(void) const { return nodeName; }
    //Gets the DAG node's output index.
    unsigned int GetDAGNodeOutIndex(void) const { return nodeOutIndex; }

    //Sets the value of this input to the given DAG node.
    void SetDAGNodeInput(const std::string & _nodeName, unsigned int nodeOutputIndex)
    {
        nodeName = _nodeName;
        nodeOutIndex = nodeOutputIndex;
    }


    bool ReadData(DataReader * data, std::string & outError) override
    {
        MaybeValue<std::string> tryStr = data->ReadString(outError);
        if (!tryStr.HasValue())
        {
            outError = "Error reading the DAG node name: " + outError;
            return false;
        }
        nodeName = tryStr.GetValue();

        MaybeValue<unsigned int> tryUInt = data->ReadUInt(outError);
        if (!tryUInt.HasValue())
        {
            outError = "Error reading the DAG node out index: " + outError;
            return false;
        }
        nodeOutIndex = tryUInt.GetValue();

        return true;
    }
    bool WriteData(DataWriter * data, std::string & outError) const override
    {
        if (!data->WriteString(nodeName, "Node Name", outError))
        {
            outError = "Error writing out the node's name, '" + nodeName + "': " + outError;
            return false;
        }
        if (!data->WriteUInt(nodeOutIndex, "Node output index", outError))
        {
            outError = "Error writing out the node's output index " +
                std::to_string(nodeOutIndex) + ": " + outError;
            return false;
        }
        return true;
    }


private:

    std::string nodeName;
    unsigned int nodeOutIndex;
};



DAGInput<float> * dIn = 0;
DAGInput<void> * dIn2 = 0;
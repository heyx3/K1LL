#pragma once

#include "../DataNode.h"
#include "ShaderInNodes.h"

//Some secondary shader input stuff.


//The world position of the vertex/fragment position.
class WorldPosNode : public DataNode
{
public:

    virtual std::string GetName(void) const override { return "worldPosNode"; }

    WorldPosNode(void) : DataNode(MakeVector(DataLine(DataNodePtr(new ObjectPosNode()), 0)), MakeVector(3))
    {

    }


protected:

    virtual void WriteMyOutputs(std::string & outCode) const override;
};
//The screen position of the vertex/fragment position.
class ScreenPosNode : public DataNode
{
public:

    virtual std::string GetName(void) const override { return "screenPosNode"; }

    ScreenPosNode(void) : DataNode(MakeVector(DataLine(DataNodePtr(new ObjectPosNode()), 0)), MakeVector(3)) { }


protected:

    virtual void WriteMyOutputs(std::string & outCode) const override;
};


//The world position of the vertex/fragment normal.
class WorldNormalNode : public DataNode
{
public:

    virtual std::string GetName(void) const override { return "worldNormalNode"; }

    WorldNormalNode(void) : DataNode(MakeVector(DataLine(DataNodePtr(new ObjectNormalNode()), 0)), MakeVector(3))
    {

    }


protected:

    virtual void WriteMyOutputs(std::string & outCode) const override;
};
//The screen position of the vertex/fragment normal.
class ScreenNormalNode : public DataNode
{
public:

    virtual std::string GetName(void) const override { return "screenNormalNode"; }

    ScreenNormalNode(void) : DataNode(MakeVector(DataLine(DataNodePtr(new ObjectNormalNode()), 0)), MakeVector(3)) { }


protected:

    virtual void WriteMyOutputs(std::string & outCode) const override;
};
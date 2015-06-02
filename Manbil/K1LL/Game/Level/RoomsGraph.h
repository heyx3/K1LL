#pragma once

#include <unordered_map>

#include "../../../Math/Lower Math/Vectors.h"
#include "../../../Graph/AStarSearch.h"
#include "../../Level Info/LevelInfo.h"


//Defines data structures for pathing through rooms in a level.

/*
struct RoomNode
{
    //Enables this class to be used in std collections that use hashes.
    size_t operator()(const RoomNode& r) const { return (size_t)r.RoomPtr; }
    bool operator==(const RoomNode& r) const { return r.RoomPtr== RoomPtr; }
    bool operator!=(const RoomNode& r) const { return r.RoomPtr!= RoomPtr; }


    LevelInfo::RoomData* RoomPtr;


    RoomNode(LevelInfo::RoomData* roomPtr = 0) : RoomPtr(roomPtr) { }
};
*/

typedef const LevelInfo::RoomData* RoomNode;


class RoomsGraph;

struct RoomEdge : public Edge<RoomNode, const RoomsGraph*>
{
    RoomEdge(void) : Edge(0, 0, 0) { }
    RoomEdge(RoomNode start, RoomNode end, const RoomsGraph* pDat) : Edge(start, end, pDat) { }
    
    virtual float GetTraversalCost(const GraphSearchGoal<RoomNode>& goal) const override;
    virtual float GetSearchCost(const GraphSearchGoal<RoomNode>& goal) const override;

private:

    bool IsHorizontalConnection(void) const;
};


class RoomsGraph : public Graph<RoomNode, RoomEdge>
{
public:

    //Defines a mapping of RoomNode instances to some value type.
    #define ROOM_NODE_MAP(valueType) std::unordered_map<RoomNode, valueType, RoomNode>

    //Indexes each room into a set of connected rooms.
    ROOM_NODE_MAP(std::vector<RoomNode>) Connections;
    //Indexes each room into its connecting rooms, which each correspond to a bool
    //    describing whether the connection is vertical or horizontal.
    ROOM_NODE_MAP(ROOM_NODE_MAP(bool)) AreConnectionsHorizontal;


    virtual void GetConnectedEdges(RoomNode startNode, std::vector<RoomEdge>& outConnections) const override;
};

typedef AStarSearch<RoomNode, RoomEdge, GraphSearchGoal<RoomNode>, const RoomsGraph*> RoomsGraphPather;
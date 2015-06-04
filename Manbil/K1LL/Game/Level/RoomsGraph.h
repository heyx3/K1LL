#pragma once

#include <unordered_map>

#include "../../../Math/Lower Math/Vectors.h"
#include "../../../Graph/AStarSearch.h"
#include "../../Level Info/LevelInfo.h"


//Defines data structures for pathing through rooms in a level.


struct RoomNode
{
    //Enables this class to be used in std collections that use hashes.
    size_t operator()(const RoomNode& r) const { return (size_t)r.Room; }
    bool operator==(const RoomNode& r) const { return r.Room == Room; }
    bool operator!=(const RoomNode& r) const { return r.Room != Room; }


    const LevelInfo::RoomData* Room;


    RoomNode(const LevelInfo::RoomData* roomPtr = 0) : Room(roomPtr) { }
};


//typedef const LevelInfo::RoomData* RoomNode;


class RoomsGraph;

struct RoomEdge : public Edge<RoomNode>
{
    RoomEdge(void) : Edge(0, 0, 0) { }
    RoomEdge(RoomNode start, RoomNode end, void* pDat = 0) : Edge(start, end, pDat) { }
    
    virtual float GetTraversalCost(const GraphSearchGoal<RoomNode>& goal) const override;
    virtual float GetSearchCost(const GraphSearchGoal<RoomNode>& goal) const override;
};


class RoomsGraph : public Graph<RoomNode, RoomEdge>
{
public:

    //Indexes each room into a set of connected rooms.
    std::unordered_map<RoomNode, std::vector<RoomNode>, RoomNode> Connections;


    virtual void GetConnectedEdges(RoomNode startNode, std::vector<RoomEdge>& outConnections) const override;
};

typedef AStarSearch<RoomNode, RoomEdge, GraphSearchGoal<RoomNode>, void*, RoomNode> RoomsGraphPather;
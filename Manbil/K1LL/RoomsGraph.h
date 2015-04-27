#pragma once

#include <unordered_map>
#include "../Math/Lower Math/Vectors.h"
#include "../Graph/AStarSearch.h"


//Defines data structures for pathing through rooms in a level.


enum ItemTypes
{
    IT_WEAPON_LIGHT,
    IT_WEAPON_HEAVY,
    IT_WEAPON_SPECIAL,
    IT_AMMO_LIGHT,
    IT_AMMO_HEAVY,
    IT_AMMO_SPECIAL,
    IT_HEALTH,
    IT_NONE,
};


struct RoomNode
{
    //Enables this class to be used in std collections that use hashes.
    unsigned int operator()(const RoomNode& r) const { return r.RoomCenter.GetHashCode(); }
    bool operator==(const RoomNode& r) const { return r.RoomCenter == RoomCenter; }
    bool operator!=(const RoomNode& r) const { return r.RoomCenter != RoomCenter; }


    Vector2u RoomCenter;
    ItemTypes ContainedItem;


    RoomNode(Vector2u roomCenter, ItemTypes containedItem)
        : RoomCenter(roomCenter), ContainedItem(containedItem) { }

    RoomNode(Vector2u minGrid, Vector2u maxGrid, ItemTypes containedItem)
        : RoomNode((minGrid + maxGrid) / 2, containedItem) { }
};


struct RoomEdge : public Edge<RoomNode>
{
    RoomEdge(RoomNode start, RoomNode end, void* pDat = 0) : Edge(start, end, pDat) { }
    
    virtual float GetTraversalCost(GraphSearchGoal<RoomNode>& goal) override;
    virtual float GetSearchCost(GraphSearchGoal<RoomNode>& goal) override;
};


class RoomsGraph : public Graph<RoomNode, RoomEdge>
{
public:

    //Indexes each room into a set of connected rooms.
    std::unordered_map<RoomNode, std::vector<RoomNode>> Connections;

    virtual void GetConnectedEdges(RoomNode startNode, std::vector<RoomEdge>& outConnections) override;
};


typedef AStarSearch<RoomNode, RoomEdge> RoomsGraphPather;
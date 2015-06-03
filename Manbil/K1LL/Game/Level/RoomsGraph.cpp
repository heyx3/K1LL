#include "RoomsGraph.h"

#include "../../Level Info/LevelInfo.h"


namespace
{
    //The importance of distance to the destination in the A* heuristic.
    const float NAV_DistScale = 0.25f;
}


float RoomEdge::GetTraversalCost(const GraphSearchGoal<RoomNode>& goal) const
{
    float baseCost = Start.Room->AverageLength;

    if (goal.SpecificEnd.HasValue())
    {
        float distSqr = End.Room->MinCornerPos.DistanceSquared(goal.SpecificEnd.GetValue().Room->MinCornerPos);
        baseCost += NAV_DistScale * distSqr;
    }

    return baseCost;
}
float RoomEdge::GetSearchCost(const GraphSearchGoal<RoomNode>& goal) const
{
    return 1.0f;
}


void RoomsGraph::GetConnectedEdges(RoomNode startNode, std::vector<RoomEdge>& outConnections) const
{
    auto nodeConns = Connections.find(startNode);
    if (nodeConns == Connections.end())
    {
        return;
    }

    outConnections.reserve(outConnections.size() + nodeConns->second.size());
    for (unsigned int i = 0; i < nodeConns->second.size(); ++i)
    {
        outConnections.push_back(RoomEdge(startNode, nodeConns->second[i], this));
    }
}
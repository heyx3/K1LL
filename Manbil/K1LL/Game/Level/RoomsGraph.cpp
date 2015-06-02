#include "RoomsGraph.h"

#include "../../Level Info/LevelInfo.h"


namespace
{
    //The importance of distance to the destination in the A* heuristic.
    const float NAV_DistScale = 0.25f;
}


float RoomEdge::GetTraversalCost(const GraphSearchGoal<RoomNode>& goal) const
{
    std::hash<RoomNode> hasher;
    float baseCost = 0.5f * (IsHorizontalConnection() ?
                                 Start->NavDifficultyHorz + End->NavDifficultyHorz :
                                 Start->NavDifficultyVert + End->NavDifficultyVert);

    if (goal.SpecificEnd.HasValue())
    {
        float distSqr = End->MinCornerPos.DistanceSquared(goal.SpecificEnd.GetValue()->MinCornerPos);
        baseCost += NAV_DistScale * distSqr;
    }

    return baseCost;
}
float RoomEdge::GetSearchCost(const GraphSearchGoal<RoomNode>& goal) const
{
    return 1.0f;
}

bool RoomEdge::IsHorizontalConnection(void) const
{
    assert(UserData->AreConnectionsHorizontal.find(Start) != UserData->AreConnectionsHorizontal.end());
    assert(UserData->AreConnectionsHorizontal.find(Start)->second.find(End) !=
           UserData->AreConnectionsHorizontal.find(Start)->second.end());

    return UserData->AreConnectionsHorizontal.find(Start)->second.find(End)->second;
}


void RoomsGraph::GetConnectedEdges(RoomNode startNode, std::vector<RoomEdge>& outConnections) const
{
    assert(Connections.find(startNode) != Connections.end());

    const std::vector<RoomNode>& nodes = Connections.find(startNode)->second;

    outConnections.reserve(outConnections.size() + nodes.size());
    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
        outConnections.push_back(RoomEdge(startNode, nodes[i], this));
    }
}
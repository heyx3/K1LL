#include "RoomsGraph.h"


float RoomEdge::GetTraversalCost(GraphSearchGoal<RoomNode>& goal)
{
    float dist = Start.RoomCenter.Distance(End.RoomCenter);

    if (goal.SpecificEnd.HasValue())
    {
        return dist + End.RoomCenter.Distance(goal.SpecificEnd.GetValue().RoomCenter);
    }
    else
    {
        return dist;
    }
}
float RoomEdge::GetSearchCost(GraphSearchGoal<RoomNode>& goal)
{
    return Start.RoomCenter.Distance(End.RoomCenter);
}

void RoomsGraph::GetConnectedEdges(RoomNode startNode, std::vector<RoomEdge>& outConnections)
{
    assert(Connections.find(startNode) != Connections.end());

    std::vector<RoomNode>& nodes = Connections[startNode];

    outConnections.reserve(outConnections.size() + nodes.size());
    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
        outConnections.push_back(RoomEdge(startNode, nodes[i]));
    }
}
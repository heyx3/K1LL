#include "LevelGraph.h"



float LevelEdge::GetTraversalCost(GraphSearchGoal<LevelNode>& goal)
{
    float dist = Start.Pos.Distance(End.Pos);

    if (goal.SpecificEnd.HasValue())
    {
        return dist + End.Pos.Distance(goal.SpecificEnd.GetValue().Pos);
    }
    else
    {
        return dist;
    }
}
float LevelEdge::GetSearchCost(GraphSearchGoal<LevelNode>& goal)
{
    return Start.Pos.Distance(End.Pos);
}

void LevelGraph::GetConnectedEdges(LevelNode startNode, std::vector<LevelEdge>& outConnections)
{
    assert(NodeConnections.find(startNode.ID) != NodeConnections.end());

    std::vector<LevelNode>& nodes = NodeConnections[startNode.ID];

    outConnections.reserve(outConnections.size() + nodes.size());
    for (auto iterator = nodes.begin(); iterator != nodes.end(); ++iterator)
    {
        outConnections.push_back(LevelEdge(startNode, *iterator, this));
    }
}
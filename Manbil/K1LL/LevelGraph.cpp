#include "LevelGraph.h"



float LevelEdge::GetTraversalCost(GraphSearchGoal<LevelNode>& goal)
{
    float dist = Start.GridPos.Distance(End.GridPos);

    if (goal.SpecificEnd.HasValue())
    {
        return dist + End.GridPos.Distance(goal.SpecificEnd.GetValue().GridPos);
    }
    else
    {
        return dist;
    }
}
float LevelEdge::GetSearchCost(GraphSearchGoal<LevelNode>& goal)
{
    return Start.GridPos.Distance(End.GridPos);
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
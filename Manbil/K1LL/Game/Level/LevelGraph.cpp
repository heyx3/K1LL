#include "LevelGraph.h"


float LevelEdge::GetTraversalCost(const GraphSearchGoal<LevelNode>& goal) const
{
    float dist = Start.Distance(End);

    if (goal.SpecificEnd.HasValue())
    {
        return dist + End.DistanceSquared(goal.SpecificEnd.GetValue());
    }
    else
    {
        return dist;
    }
}
float LevelEdge::GetSearchCost(const GraphSearchGoal<LevelNode>& goal) const
{
    //The blocks should be either adjacent or diagonal to each other.

    if (Start.x == End.x || Start.y == End.y)
    {
        assert(Start.ManhattanDistance(End) == 1);
        return 1.0f;
    }

    assert(Start.ManhattanDistance(End) == 2);
    return 1.41421356f;
}

void LevelGraph::GetConnectedEdges(LevelNode startNode, std::vector<LevelEdge>& outConnections) const
{
    assert(startNode.x < LevelGrid.GetWidth() &&
           startNode.y < LevelGrid.GetHeight());

    bool atMinX = (startNode.x == 0),
         atMinY = (startNode.y == 0),
         atMaxX = (startNode.x == LevelGrid.GetWidth() - 1),
         atMaxY = (startNode.y == LevelGrid.GetHeight() - 1);

    outConnections.reserve(8);
    if (!atMinX)
    {
        Vector2u lessX = startNode.LessX();

        if (IsFree(lessX))
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(lessX)));

            //Check the corners.
            if (!atMinY && IsFree(lessX.LessY()) && IsFree(startNode.LessY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(lessX.LessY())));
            }
            if (!atMaxY && IsFree(lessX.MoreY()) && IsFree(startNode.MoreY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(lessX.MoreY())));
            }
        }
    }
    if (!atMaxX)
    {
        Vector2u moreX = startNode.MoreX();

        if (IsFree(moreX))
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(moreX)));

            //Check the corners.
            if (!atMinY && IsFree(moreX.LessY()) && IsFree(startNode.LessY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(moreX.LessY())));
            }
            if (!atMaxY && IsFree(moreX.MoreY()) && IsFree(startNode.MoreY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(moreX.MoreY())));
            }
        }
    }
    if (!atMinY && IsFree(startNode.LessY()))
    {
        outConnections.push_back(LevelEdge(startNode, LevelNode(startNode.LessY())));
    }
    if (!atMaxY && IsFree(startNode.MoreY()))
    {
        outConnections.push_back(LevelEdge(startNode, LevelNode(startNode.MoreY())));
    }
}
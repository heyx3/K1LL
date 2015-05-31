#include "LevelGraph.h"


float LevelEdge::GetTraversalCost(const GraphSearchGoal<LevelNode>& goal) const
{
    float dist = Start.GridPos.Distance(End.GridPos);

    if (goal.SpecificEnd.HasValue())
    {
        return dist + End.GridPos.DistanceSquared(goal.SpecificEnd.GetValue().GridPos);
    }
    else
    {
        return dist;
    }
}
float LevelEdge::GetSearchCost(const GraphSearchGoal<LevelNode>& goal) const
{
    //The blocks should be either adjacent or diagonal to each other.

    if (Start.GridPos.x == End.GridPos.x || Start.GridPos.y == End.GridPos.y)
    {
        assert(Start.GridPos.ManhattanDistance(End.GridPos) == 1);
        return 1.0f;
    }

    assert(Start.GridPos.ManhattanDistance(End.GridPos) == 2);
    return 1.41421356f;
}

void LevelGraph::GetConnectedEdges(LevelNode startNode, std::vector<LevelEdge>& outConnections) const
{
    assert(startNode.GridPos.x < LevelGrid.GetWidth() &&
           startNode.GridPos.y < LevelGrid.GetHeight());

    bool atMinX = (startNode.GridPos.x == 0),
         atMinY = (startNode.GridPos.y == 0),
         atMaxX = (startNode.GridPos.x == LevelGrid.GetWidth() - 1),
         atMaxY = (startNode.GridPos.y == LevelGrid.GetHeight() - 1);

    outConnections.reserve(8);
    if (!atMinX)
    {
        Vector2u lessX = startNode.GridPos.LessX();

        if (IsFree(lessX))
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(lessX)));

            //Check the corners.
            if (!atMinY && IsFree(lessX.LessY()) && IsFree(startNode.GridPos.LessY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(lessX.LessY())));
            }
            if (!atMaxY && IsFree(lessX.MoreY()) && IsFree(startNode.GridPos.MoreY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(lessX.MoreY())));
            }
        }
    }
    if (!atMaxX)
    {
        Vector2u moreX = startNode.GridPos.MoreX();

        if (IsFree(moreX))
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(moreX)));

            //Check the corners.
            if (!atMinY && IsFree(moreX.LessY()) && IsFree(startNode.GridPos.LessY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(moreX.LessY())));
            }
            if (!atMaxY && IsFree(moreX.MoreY()) && IsFree(startNode.GridPos.MoreY()))
            {
                outConnections.push_back(LevelEdge(startNode, LevelNode(moreX.MoreY())));
            }
        }
    }
    if (!atMinY && IsFree(startNode.GridPos.LessY()))
    {
        outConnections.push_back(LevelEdge(startNode, LevelNode(startNode.GridPos.LessY())));
    }
    if (!atMaxY && IsFree(startNode.GridPos.MoreY()))
    {
        outConnections.push_back(LevelEdge(startNode, LevelNode(startNode.GridPos.MoreY())));
    }
}
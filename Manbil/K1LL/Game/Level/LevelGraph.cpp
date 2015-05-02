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

        if (LevelGrid[lessX] != BT_WALL)
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(lessX)));
        }

        if (!atMinY && LevelGrid[lessX.LessY()] != BT_WALL)
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(lessX.LessY())));
        }
        if (!atMaxY && LevelGrid[lessX.MoreY()] != BT_WALL)
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(lessX.MoreY())));
        }
    }
    if (!atMaxX)
    {
        Vector2u moreX = startNode.GridPos.MoreX();

        if (LevelGrid[moreX] != BT_WALL)
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(moreX)));
        }

        if (!atMinY && LevelGrid[moreX.LessY()] != BT_WALL)
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(moreX.LessY())));
        }
        if (!atMaxY && LevelGrid[moreX.MoreY()] != BT_WALL)
        {
            outConnections.push_back(LevelEdge(startNode, LevelNode(moreX.MoreY())));
        }
    }
    if (!atMinY && LevelGrid[startNode.GridPos.LessY()] != BT_WALL)
    {
        outConnections.push_back(LevelEdge(startNode, LevelNode(startNode.GridPos.LessY())));
    }
    if (!atMaxY && LevelGrid[startNode.GridPos.MoreY()] != BT_WALL)
    {
        outConnections.push_back(LevelEdge(startNode, LevelNode(startNode.GridPos.MoreY())));
    }
}
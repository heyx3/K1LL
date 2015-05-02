#pragma once

#include <unordered_map>
#include "../../../Math/Lower Math/Array2D.h"
#include "../../../Math/Lower Math/Vectors.h"

#include "../../../Graph/AStarSearch.h"
#include "../../Level Info/RoomInfo.h"


//Defines data structures for pathing through grid spots in a level.


struct LevelNode
{
    Vector2u GridPos;
    LevelNode(Vector2u gridPos) : GridPos(gridPos) { }
};


struct LevelEdge : public Edge<LevelNode>
{
    LevelEdge(LevelNode start, LevelNode end, void* pDat = 0) : Edge(start, end, pDat) { }
    
    virtual float GetTraversalCost(GraphSearchGoal<LevelNode>& goal) override;
    virtual float GetSearchCost(GraphSearchGoal<LevelNode>& goal) override;
};


class LevelGraph : public Graph<LevelNode, LevelEdge>
{
public:

    const Array2D<BlockTypes>& LevelGrid;

    LevelGraph(const Array2D<BlockTypes>& levelGrid) : LevelGrid(levelGrid) { }

    
    virtual void GetConnectedEdges(LevelNode startNode, std::vector<LevelEdge>& outConnections) override;
};


typedef AStarSearch<LevelNode, LevelEdge> LevelGraphPather;
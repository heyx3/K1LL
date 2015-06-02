#pragma once

#include <unordered_map>
#include "../../../Math/Lower Math/Array2D.h"
#include "../../../Math/Lower Math/Vectors.h"

#include "../../../Graph/AStarSearch.h"
#include "../../Level Info/RoomInfo.h"


//Defines data structures for pathing through grid spots in a level.


typedef Vector2u LevelNode;


struct LevelEdge : public Edge<LevelNode>
{
    LevelEdge(void) : Edge(LevelNode(), LevelNode(), 0) { }
    LevelEdge(LevelNode start, LevelNode end, void* pDat = 0) : Edge(start, end, pDat) { }
    
    virtual float GetTraversalCost(const GraphSearchGoal<LevelNode>& goal) const override;
    virtual float GetSearchCost(const GraphSearchGoal<LevelNode>& goal) const override;
};


class LevelGraph : public Graph<LevelNode, LevelEdge>
{
public:

    const Array2D<BlockTypes>& LevelGrid;


    LevelGraph(const Array2D<BlockTypes>& levelGrid) : LevelGrid(levelGrid) { }

    
    virtual void GetConnectedEdges(LevelNode startNode,
                                   std::vector<LevelEdge>& outConnections) const override;


private:

    bool IsFree(Vector2u pos) const { return LevelGrid[pos] != BT_WALL; }
};


typedef AStarSearch<LevelNode, LevelEdge, GraphSearchGoal<LevelNode>, void*, LevelNode> LevelGraphPather;
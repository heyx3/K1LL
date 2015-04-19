#pragma once

#include <unordered_map>
#include "../Math/Lower Math/Vectors.h"
#include "../Graph/AStarSearch.h"


struct LevelNode
{
    unsigned int ID;
    Vector2f GridPos;

    LevelNode(unsigned int id, Vector2f gridPos) : ID(id), GridPos(gridPos) { }
};


class LevelGraph;

struct LevelEdge : Edge<LevelNode>
{
    LevelEdge(LevelNode start, LevelNode end, void* graph) : Edge(start, end, graph) { }
    
    virtual float GetTraversalCost(GraphSearchGoal<LevelNode>& goal) override;
    virtual float GetSearchCost(GraphSearchGoal<LevelNode>& goal) override;
};


class LevelGraph : public Graph<LevelNode, LevelEdge>
{
public:

    std::unordered_map<unsigned int, std::vector<LevelNode>> NodeConnections;


    virtual void GetConnectedEdges(LevelNode startNode, std::vector<LevelEdge>& outConnections) override;
};


typedef AStarSearch<LevelNode, LevelEdge> LevelGraphPather;
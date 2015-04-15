#pragma once

#include <unordered_map>
#include "../Math/Lower Math/Vectors.h"
#include "../Graph/AStarSearch.h"


struct LevelNode
{
    unsigned int ID;
    Vector2f Pos;

    LevelNode(unsigned int id, Vector2f pos) : ID(id), Pos(pos) { }
};


class LevelGraph;

struct LevelEdge : Edge<LevelNode, const LevelGraph*>
{
    LevelEdge(LevelNode start, LevelNode end, const LevelGraph* graph) : Edge(start, end, graph) { }
    
    virtual float GetTraversalCost(GraphSearchGoal<LevelNode>& goal) override;
    virtual float GetSearchCost(GraphSearchGoal<LevelNode>& goal) override;
};


class LevelGraph : public Graph<LevelNode, LevelEdge>
{
public:

    std::unordered_map<unsigned int, std::vector<LevelNode>> NodeConnections;


    virtual void GetConnectedEdges(LevelNode startNode, std::vector<LevelEdge>& outConnections) override;
};


typedef AStarSearch<LevelNode, LevelEdge, GraphSearchGoal<LevelNode>, LevelGraph*> LevelGraphPather;
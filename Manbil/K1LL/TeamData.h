#pragma once

#include "../Math/Lower Math/Vectors.h"


enum Team
{
    T_ONE,
    T_TWO,
};


//Information about a single team.
struct TeamData
{
public:

    //Information about each team.
    static TeamData TOne, TTwo;

    static const TeamData& GetData(Team team) { return (team == T_ONE) ? TOne : TTwo; }


    Vector3f TeamColor;
    unsigned int TeamMeshIndex;


private:

    TeamData(void) { }
};
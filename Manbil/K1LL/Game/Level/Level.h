#pragma once

#include "../../../Rendering/Basic Rendering/RenderInfo.h"

#include "../../Level Info/LevelInfo.h"
#include "LevelGraph.h"
#include "RoomsGraph.h"

#include "../Players/Player.h"
#include "../Actor.h"


//The game level.
//Each grid spot is exactly 1 square meter.
class Level
{
public:

    Array2D<BlockTypes> BlockGrid;

    LevelGraph NavGraph;
    RoomsGraph RoomGraph;
    std::vector<LevelInfo::UIntBox> RoomBounds;

    std::vector<PlayerPtr> Players;
    std::vector<ActorPtr> Actors;


    //If there was an error initializing the level, outputs an error message to the given string.
    Level(const LevelInfo& level, std::string& errorMsg);

    void Update(float elapsed);
    void Render(float elapsed, const RenderInfo& info);


    enum RaycastResults
    {
        RR_FLOOR,
        RR_WALL,
        RR_CEILING,
    };

    //Casts the given ray into this level.
    //Outputs the position/time of the hit into "hitPos" and "hitT" respectively.
    //Also returns what kind of surface was hit.
    RaycastResults CastWallRay(Vector3f start, Vector3f dir, Vector3f& hitPos, float& hitT);
};
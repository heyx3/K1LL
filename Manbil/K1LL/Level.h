#pragma once

#include "Room.h"
#include "LevelInfo.h"
#include "Game\LevelGraph.h"
#include "RoomsGraph.h"


//The game level.
class Level
{
public:

    Array2D<BlockTypes> BlockGrid;
    LevelGraph NavGraph;
    RoomsGraph RoomGraph;


    //If there was an error initializing the level, outputs an error message to the given string.
    Level(const LevelInfo& level, std::string& errorMsg);

    void Update(float elapsed);
    void Render(float elapsed, const RenderInfo& info);
    
    //Puts the given player into the given room.
    Room* PutInRoom(PlayerPtr player);

    //Puts the given actor into the given room.
    //Room* PutInRoom(ActorPtr actor);


    //TODO: Add some block-querying stuff.
};
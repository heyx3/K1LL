#pragma once

#include "Room.h"
#include "LevelGraph.h"


//The game level. Only one may exist at a time.
class Level
{
public:

    std::vector<RoomPtr> Rooms;
    LevelGraph NavGraph;


    void Update(float elapsed);
    void Render(float elapsed, const RenderInfo& info);
    
    //Puts the given player into the given room.
    Room* PutInRoom(PlayerPtr player);

    //Puts the given actor into the given room.
    //Room* PutInRoom(ActorPtr actor);


    //TODO: Add some block-querying stuff.


private:

    Level(void) { }
};
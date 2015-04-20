#pragma once

#include <memory>

#include "../../Rendering/Basic Rendering/RenderInfo.h"
#include "../../Math/Shapes.hpp"

#include "../Content/Constants.h"
#include "../Content/ActorContent.h"


class Room;

//A non-player entity that can update and render itself.
class Actor
{
public:

    Room* CurrentRoom;
    Vector3f Pos;


    Actor(Vector3f pos, Room* startingRoom) : Pos(pos), CurrentRoom(startingRoom) { }
    virtual ~Actor(void) { }
    
    
    virtual void Update(float elapsedSeconds) = 0;
    virtual void Render(float elapsedSeconds, const RenderInfo& info) = 0;
};

typedef std::shared_ptr<Actor> ActorPtr;
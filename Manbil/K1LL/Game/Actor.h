#pragma once

#include <memory>

#include "../../Rendering/Basic Rendering/RenderInfo.h"
#include "../../Math/Shapes.hpp"

#include "../Content/LevelConstants.h"
#include "../Content/ActorContent.h"


class Level;

//A non-player entity that can update and render itself.
class Actor
{
public:

    Actor(Level* theLevel) : lvl(theLevel) { }
    virtual ~Actor(void) { }
    

    Level* GetLevel(void) const { return lvl; }
    
    //Returns whether this actor should be removed from the level.
    virtual bool Update(float elapsedSeconds) = 0;

    virtual void Render(float elapsedSeconds, const RenderInfo& info) = 0;


private:

    Level* lvl;
};

typedef std::shared_ptr<Actor> ActorPtr;
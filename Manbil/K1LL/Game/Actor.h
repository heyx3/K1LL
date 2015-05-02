#pragma once

#include <memory>

#include "../../Rendering/Basic Rendering/RenderInfo.h"
#include "../../Math/Shapes.hpp"

#include "../Content/Constants.h"
#include "../Content/ActorContent.h"


class Level;

//A non-player entity that can update and render itself.
class Actor
{
public:

    virtual ~Actor(void) { }
    
    
    //Returns whether this actor should be destroyed.
    virtual bool Update(Level* theLevel, float elapsedSeconds) = 0;

    virtual void Render(Level* theLevel, float elapsedSeconds, const RenderInfo& info) = 0;
};

typedef std::shared_ptr<Actor> ActorPtr;
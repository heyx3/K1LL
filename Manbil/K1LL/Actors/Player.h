#pragma once

#include "Actor.h"
#include "../TeamData.h"


//An entity that participates in the game.
class Player
{
public:
    
    Team MyTeam;

    Vector2f Pos, Velocity;
    Vector3f LookDir;


    Player(Vector2f pos);


    inline Capsule GetCollision3D(void) const
    {
        const float radius = Constants::Instance.PlayerCollisionRadius;
        return Capsule(Vector3f(Pos.x, Pos.y, 0.0),
                       Vector3f(Pos.x, Pos.y, Constants::Instance.PlayerHeight - radius),
                       radius);
    }
    inline Circle GetCollision2D(void) const
    {
        return Circle(Pos, Constants::Instance.PlayerCollisionRadius);
    }


    //Tells this player to push back against the given wall it intersects with.
    //Takes in the elapsed time this frame.
    void PushOffWall(const Box2D& wall, float elapsedSeconds);


    //Child classes should call this AFTER doing their own update logic.
    //Behavior: updates the velocity and position given the acceleration.
    virtual void Update(float elapsedSeconds);
    //Default behavior: renders the player model for this player's team,
    //    positioned and oriented like this player.
    virtual void Render(float elapsedSeconds, const RenderInfo& info);


protected:

    //The world-space horizontal acceleration this turn. Resets to 0 at the end of every update.
    Vector2f Acceleration;
};


typedef std::shared_ptr<Player> PlayerPtr;
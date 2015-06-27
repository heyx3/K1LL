#pragma once

#include "../../Actor.h"


//Because bullets get created and destroyed constantly, we will pool them.
//A single actor contains a single buffer holding the puncher bullets.


//A bullet shot by a Puncher weapon.
struct PuncherBullet
{
public:

    Vector3f Pos, Velocity;


    PuncherBullet(void) : PuncherBullet(Vector3f(), Vector3f()) { }
    PuncherBullet(Vector3f pos, Vector3f vel) : Pos(pos), Velocity(vel) { }


    //Returns whether this bullet should be destroyed.
    bool Update(Level* theLevel, float elapsedSeconds);
    void Render(Level* theLevel, float elapsedSeconds, const RenderInfo& info);
};


//Pre-allocates all the Puncher bullets that may be needed.
//Only one instance of this class exists at a time.
class PuncherBulletPool : public Actor
{
public:

    //Creates a new pool. The previous pool is deleted if it exists.
    static ActorPtr CreatePool(Level* lvl);

    static PuncherBulletPool* GetInstance(void) { return (PuncherBulletPool*)instance.get(); }
    ActorPtr GetInstanceSharedPtr(void) { return instance; }

    //Always returns a new, valid bullet.
    static PuncherBullet* NewBullet(Vector3f pos, Vector3f velocity, Level* lvl);

    
    PuncherBulletPool(Level* lvl);
    ~PuncherBulletPool(void);
    

    virtual bool Update(float elapsedSeconds) override;
    virtual void Render(float elapsedSeconds, const RenderInfo& info) override;


private:
    
    static ActorPtr instance;


    PuncherBullet* NewBulletInstance(Vector3f pos, Vector3f velocity, Level* lvl);


    //Arrays for bullet allocation.
    bool* isAllocated;
    PuncherBullet* bullets;

    unsigned int nextBullet = 0;

    //A collection of the currently-allocated bullets for updating/rendering.
    std::vector<unsigned int> activeBullets;
};
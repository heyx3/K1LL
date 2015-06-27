#pragma once

#include "../../../Math/Shapes.hpp"
#include "../../../Rendering/Basic Rendering/RenderInfo.h"

#include "../../Content/LevelConstants.h"
#include "../Level/Level.h"



//An entity that participates in the game.
class Player
{
public:
    
    Level* Lvl;
    Team MyTeam = T_ONE;

    Vector2f Pos, Velocity;
    Vector3f LookDir;

    Weapon::Ptr Weapons[3];
    unsigned int Ammo[3];


    Player(Level* level, Vector2f pos, Weapon::Ptr weapons[3]);
    virtual ~Player(void) { }


    inline Sphere GetBroadCollision3D(void) const
    {
        const float radius = Mathf::Max(LevelConstants::Instance.PlayerCollisionRadius,
                                        LevelConstants::Instance.PlayerHeight);
        return Sphere(Vector3f(Pos.x, Pos.y, LevelConstants::Instance.PlayerHeight * 0.5f),
                      radius);
    }
    inline Capsule GetCollision3D(void) const
    {
        const float radius = LevelConstants::Instance.PlayerCollisionRadius;
        return Capsule(Vector3f(Pos.x, Pos.y, 0.0),
                       Vector3f(Pos.x, Pos.y, LevelConstants::Instance.PlayerHeight - radius),
                       radius);
    }
    inline Circle GetCollision2D(void) const
    {
        return Circle(Pos, LevelConstants::Instance.PlayerCollisionRadius);
    }


    //Gets whether this player is currently holding down the "fire" input.
    bool IsFiring(void) const { return firedLastFrame; }

    WeaponTypes GetCurrentWeaponType(void) const { return currentWeapon; }
    Weapon* GetCurrentWeapon(void) { return Weapons[currentWeapon].get(); }
    const Weapon* GetCurrentWeapon(void) const { return Weapons[currentWeapon].get(); }

    void SetCurrentWeaponType(WeaponTypes newWeapon);

    //Gets the world-space position/direction the player's weapon has, in that order.
    std::pair<Vector3f, Vector3f> GetWeaponPosAndDir(void) const;


    //Child classes should call this AFTER doing their own update logic.
    //Default behavior: updates position/velocity based on acceleration and possibly fires weapons.
    virtual void Update(float elapsedSeconds);
    //Default behavior: renders the player model for this player's team,
    //    positioned and oriented like this player.
    virtual void Render(float elapsedSeconds, const RenderInfo& info);


protected:

    //The world-space horizontal acceleration this turn. Resets to 0 at the end of every update.
    Vector2f Acceleration;
    //Whether to fire the weapon next update. Resets to "false" at the end of every update.
    bool Fire;


private:

    bool firedLastFrame = false;
    WeaponTypes currentWeapon;


    //Tries to move position based on the velocity and the given time step.
    //If a wall is hit, the player reacts accordingly.
    void TryMove(float timeStep);
};


typedef std::shared_ptr<Player> PlayerPtr;
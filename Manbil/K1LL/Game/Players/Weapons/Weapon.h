#pragma once

#include <memory>

#include "../../../../Rendering/Basic Rendering/RenderInfo.h"



enum WeaponTypes
{
    WT_LIGHT,
    WT_HEAVY,
    WT_SPECIAL,
};


class Level;
class Player;


class Weapon
{
public:

    typedef std::shared_ptr<Weapon> Ptr;


    Player* Owner;
    Level& Lvl;


    Weapon(WeaponTypes thisType, Level& level) : Owner(0), Lvl(level), type(thisType) { }
    virtual ~Weapon(void) { }


    bool IsFiring(void) const { return isFiring; }
    WeaponTypes GetType(void) const { return type; }


    //Should be called when the player starts to fire the gun.
    virtual void StartFire(void) { isFiring = true; }
    //Should be called once the player stops firing the gun.
    virtual void StopFire(void) { isFiring = false; }
    
    //Updates this weapon.
    virtual void Update(float elapsed) = 0;
    //Renders this weapon at the given position and facing the given direction.
    virtual void Render(Vector3f pos, Vector3f dir, const RenderInfo& info) const = 0;


private:

    bool isFiring = false;

    WeaponTypes type;
};
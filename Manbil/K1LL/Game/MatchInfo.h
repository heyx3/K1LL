#pragma once

#include "Players/Weapons/Weapon.h"


enum Team
{
    T_ONE = 0,
    T_TWO,
};


//Information about a specific match.
struct MatchInfo
{
public:

    Vector3f TeamColors[2];
    unsigned int TeamPlayerMeshIndex[2];

    //Creates a specific type of weapon for the given player.
    typedef Weapon::Ptr(*WeaponFactory)(Level& lvl);
    WeaponFactory LightWeapon, HeavyWeapon, SpecialWeapon;


    MatchInfo(Vector3f team1Color, Vector3f team2Color,
              unsigned int team1PlayerMesh, unsigned int team2PlayerMesh,
              WeaponFactory light, WeaponFactory heavy, WeaponFactory special)
        : LightWeapon(light), HeavyWeapon(heavy), SpecialWeapon(special)
    {
        TeamColors[T_ONE] = team1Color;
        TeamColors[T_TWO] = team2Color;
        TeamPlayerMeshIndex[T_ONE] = team1PlayerMesh;
        TeamPlayerMeshIndex[T_TWO] = team2PlayerMesh;
    }
};
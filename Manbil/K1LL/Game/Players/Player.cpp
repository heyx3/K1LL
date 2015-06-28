#include "Player.h"

#include "../../Content/ActorContent.h"

#include "../../Content/WeaponConstants.h"



Player::Player(Level* level, Vector2f pos, Weapon::Ptr weapons[3])
    : Lvl(level), LookDir(LevelConstants::Instance.PlayerStartLookDir),
      Pos(pos), currentWeapon(WT_LIGHT)
{
    Weapons[WT_LIGHT] = weapons[WT_LIGHT];
    Weapons[WT_LIGHT]->Owner = this;

    Weapons[WT_HEAVY] = weapons[WT_HEAVY];
    Weapons[WT_HEAVY]->Owner = this;

    Weapons[WT_SPECIAL] = weapons[WT_SPECIAL];
    Weapons[WT_SPECIAL]->Owner = this;
}

void Player::SetCurrentWeaponType(WeaponTypes newType)
{
    if (firedLastFrame)
    {
        GetCurrentWeapon()->StopFire();
        firedLastFrame = false;
    }

    currentWeapon = newType;
}

std::pair<Vector3f, Vector3f> Player::GetWeaponPosAndDir(void) const
{
    Quaternion rot(Vector3f(0.0f, 0.0f, 1.0f), atan2f(LookDir.y, LookDir.x));
    Vector3f offset = rot.Rotated(WeaponConstants::Instance.WeaponOffset);
    return std::pair<Vector3f, Vector3f>(Vector3f(Pos, 0.0f) + offset, LookDir);
}

void Player::Update(float elapsed)
{
    //Add friction into the acceleration.

    //If the player isn't accelerating, slow down his velocity completely.
    if (Acceleration.x == 0.0f && Acceleration.y == 0.0f)
    {
        if (Velocity.x != 0.0f || Velocity.y != 0.0f)
        {
            Acceleration = -Velocity * LevelConstants::Instance.PlayerFriction;
        }
    }
    else
    {
        //Split the acceleration and velocity into "forward" and "side" components.
        Vector2f forward = LookDir.XY().Normalized(),
                 side = Vector2f(forward.y, forward.x),
                 forwardA = forward * forward.Dot(Acceleration),
                 forwardV = forward * forward.Dot(Velocity),
                 sideA = side * side.Dot(Acceleration),
                 sideV = side * side.Dot(Velocity);

        //Add friction to any velocity components that go against the acceleration.
        if (forwardA.Dot(forwardV) < 0.0f && (abs(forwardV.x) > 0.00001f || abs(forwardV.y) > 0.00001f))
        {
            Acceleration -= forwardV.Normalized() * LevelConstants::Instance.PlayerFriction;
        }
        if (sideA.Dot(sideV) < 0.0f && (abs(sideV.x) > 0.00001f || abs(sideV.y) > 0.00001f))
        {
            Acceleration -= sideV.Normalized() * LevelConstants::Instance.PlayerFriction;
        }
    }


    //Update velocity and acceleration.
    Velocity += Acceleration * elapsed;
    Acceleration = Vector2f();

    //Constrain the velocity.
    float speedSqr = Velocity.LengthSquared();
    if (speedSqr > LevelConstants::Instance.PlayerMaxSpeed * LevelConstants::Instance.PlayerMaxSpeed)
    {
        Velocity = (Velocity / sqrtf(speedSqr)) * LevelConstants::Instance.PlayerMaxSpeed;
    }

    //Update position.
    TryMove(elapsed);


    //Handle weapons.
    if (Fire)
    {
        if (!firedLastFrame)
        {
            firedLastFrame = true;
            GetCurrentWeapon()->StartFire();
        }
        Fire = false;
    }
    else if (firedLastFrame)
    {
        GetCurrentWeapon()->StopFire();
        firedLastFrame = false;
    }
    GetCurrentWeapon()->Update(elapsed);
}
void Player::TryMove(float timeStep)
{
    Vector2f delta = Velocity * timeStep;
    Vector2f signDelta((float)Mathf::Sign(delta.x), (float)Mathf::Sign(delta.y));

    //Check the X and Y components individually for wall collisions.

    assert(LevelConstants::Instance.PlayerCollisionRadius <= 1.0f);

    Vector2f movePos = Pos + delta,
             moveMax = movePos +
                       Vector2f(signDelta.x * LevelConstants::Instance.PlayerCollisionRadius,
                                signDelta.y * LevelConstants::Instance.PlayerCollisionRadius);
    Vector2i movePosI = ToV2i(movePos),
             moveMaxI = ToV2i(moveMax);

    //X movement.
    if (Lvl->IsGridPosBlocked(Vector2i(moveMaxI.x, (int)Pos.y)))
    {
        //Cancel out horizontal velocity and move to the side of the wall.
        float targetX = (float)moveMaxI.x + 0.5f + (-signDelta.x * 0.5f) +
                        (-signDelta.x * LevelConstants::Instance.PlayerCollisionRadius);
        delta.x = targetX - Pos.x;
        Velocity.x = 0.0f;
    }
    //Y movement.
    if (Lvl->IsGridPosBlocked(Vector2i((int)Pos.x, moveMaxI.y)))
    {
        //Cancel out vertical velocity and move to the side of the wall.
        float targetY = (float)moveMaxI.y + 0.5f + (-signDelta.y * 0.5f) +
                        (-signDelta.y * LevelConstants::Instance.PlayerCollisionRadius);
        delta.y = targetY - Pos.y;
        Velocity.y = 0.0f;
    }

    //TODO: Handle collisions along corners.

    //Finally, apply the movement.
    Pos += delta;
}

void Player::Render(float elapsed, const RenderInfo& info)
{
    ActorContent::Instance.RenderPlayer(Pos, LookDir,
                                        Lvl->MatchData.TeamColors[MyTeam],
                                        Lvl->MatchData.TeamPlayerMeshIndex[MyTeam],
                                        info);

    
    //Render the weapon.
    auto posAndDir = GetWeaponPosAndDir();
    GetCurrentWeapon()->Render(posAndDir.first, posAndDir.second, info);
}
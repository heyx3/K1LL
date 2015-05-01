#include "Player.h"

#include "../Content/ActorContent.h"


Player::Player(Vector2f pos)
    : LookDir(1.0f, 0.0f, 0.0f)
{

}

void Player::PushOffWall(const Box2D& wall, float seconds)
{
    Vector2f deltaPos = Velocity * seconds,
             oldPos = Pos - deltaPos;

    //Figure out whether this player is hitting an X face or a Y face.
    bool hittingXFace;
    bool touchedX = Mathf::Abs(Pos.x - wall.GetCenterX()) <
                    ((wall.GetXSize() * 0.5f) + Constants::Instance.PlayerCollisionRadius),
         touchedY = Mathf::Abs(Pos.y - wall.GetCenterY()) <
                    ((wall.GetYSize() * 0.5f) + Constants::Instance.PlayerCollisionRadius);
    if (touchedX && !touchedY)
    {
        hittingXFace = true;
    }
    else if (touchedY && !touchedX)
    {
        hittingXFace = false;
    }
    else
    {
        //Use velocity to decide.
        if (Mathf::Abs(Velocity.x) > Mathf::Abs(Velocity.y))
        {
            hittingXFace = true;
        }
        else
        {
            hittingXFace = false;
        }
    }

    //Move the player to be against the face, and remove any velocity heading towards it.
    unsigned int axisTouching;
    float targetPlayerPos;
    Vector2f wallNormal;
    if (hittingXFace)
    {
        if (Pos.x > wall.GetCenterX())
        {
            targetPlayerPos = wall.GetXMax() + Constants::Instance.PlayerCollisionRadius;
            wallNormal = Vector2f(1.0f, 0.0f);
            axisTouching = 0;
        }
        else
        {
            targetPlayerPos = wall.GetXMin() - Constants::Instance.PlayerCollisionRadius;
            wallNormal = Vector2f(-1.0f, 0.0f);
            axisTouching = 0;
        }
    }
    else
    {
        if (Pos.y > wall.GetCenterY())
        {
            targetPlayerPos = wall.GetYMax() + Constants::Instance.PlayerCollisionRadius;
            wallNormal = Vector2f(0.0f, 1.0f);
            axisTouching = 1;
        }
        else
        {
            targetPlayerPos = wall.GetYMin() - Constants::Instance.PlayerCollisionRadius;
            wallNormal = Vector2f(0.0f, -1.0f);
            axisTouching = 1;
        }
    }

    //Set the new position.
    auto posAgainstWall = Geometryf::GetPointOnLineAtValue(oldPos, deltaPos,
                                                           axisTouching, targetPlayerPos);
    Pos = posAgainstWall.Point;

    //Set the new velocity.
    Vector2f badVelocity = wallNormal * Mathf::Max(0.0f, -wallNormal.Dot(Velocity));
    Velocity -= badVelocity;
}

void Player::Update(float elapsed)
{
    Velocity += Acceleration * elapsed;

    float speedSqr = Velocity.LengthSquared();
    if (speedSqr > Constants::Instance.PlayerMaxSpeed * Constants::Instance.PlayerMaxSpeed)
    {
        Velocity = (Velocity / sqrtf(speedSqr)) * Constants::Instance.PlayerMaxSpeed;
    }

    Pos += Velocity * elapsed;
}
void Player::Render(float elapsed, const RenderInfo& info)
{
    const TeamData& dat = TeamData::GetData(MyTeam);
    ActorContent::Instance.RenderPlayer(Pos, LookDir, dat.TeamColor, dat.TeamMeshIndex, info);
}
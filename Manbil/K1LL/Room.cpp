#include "Room.h"

#include "Level.h"


namespace
{
    //Redirects the player's velocity after he just moved into the given wall.
    void BounceOffWall(PlayerPtr player, Box2D wall, float elapsedTime)
    {
        Vector2f newPos = player->Pos;
        Vector2f vel = player->Velocity,
                       deltaPos = vel * elapsedTime;
        Vector2f oldPos = oldPos - (vel * elapsedTime);

        //Figure out whether the player is hitting the wall's X faces or its Y faces.
        bool hittingXFace;
        bool touchedX = Mathf::Abs(newPos.x - wall.GetCenterX()) <
                        ((wall.GetXSize() * 0.5f) + Constants::Instance.PlayerCollisionRadius),
             touchedY = Mathf::Abs(newPos.y - wall.GetCenterY()) <
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
            if (Mathf::Abs(vel.x) > Mathf::Abs(vel.y))
            {
                hittingXFace = true;
            }
            else
            {
                hittingXFace = false;
            }
        }

        //Move the player to be against the wall, and make sure his velocity is parallel to the wall.
        unsigned int axisTouching;
        float targetPlayerPos;
        Vector2f wallNormal;
        if (hittingXFace)
        {
            if (newPos.x > wall.GetCenterX())
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
            if (newPos.y > wall.GetCenterY())
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

        //Set the new pos.
        auto posAgainstWall = Geometryf::GetPointOnLineAtValue(oldPos, deltaPos,
                                                               axisTouching, targetPlayerPos);
        player->Pos = posAgainstWall.Point;

        //Set the new velocity.
        Vector2f velocityBad = wallNormal * wallNormal.Dot(vel),
                 velocityGood = vel - velocityBad;
        player->Velocity = velocityGood;
    }
}

Room::Room(Vector2u _pos, const RoomInfo& info)
    : pos(_pos), baseInfo(info),
      bounds((float)_pos.x, (float)_pos.y,
             ToV2f(info.RoomGrid.GetDimensions()))
{

}

void Room::Update(float elapsedSeconds)
{
    int playerGridSize = 1 + (int)(2.0f * Constants::Instance.PlayerCollisionRadius);
    for (unsigned int i = 0; i < players.size(); ++i)
    {
        players[i]->Update(elapsedSeconds);

        //Check for any collisions between this player and the walls.
        Vector2f relativePos = players[i]->Pos - bounds.GetMinCorner();
        Vector2i minGrid = relativePos.Floored(),
                 maxGrid = minGrid + Vector2i(playerGridSize, playerGridSize);
        for (int x = minGrid.x; x <= maxGrid.x; ++x)
        {
            if (x > 0 && x < baseInfo.RoomGrid.GetWidth())
            {
                unsigned int xU = (unsigned int)x;

                for (int y = minGrid.y; y <= maxGrid.y; ++y)
                {
                    if (y > 0 && y < baseInfo.RoomGrid.GetHeight())
                    {
                        unsigned int yU = (unsigned int)y;

                        if (baseInfo.RoomGrid[Vector2u(xU, yU)] == BT_WALL)
                        {
                            BounceOffWall(players[i], Box2D(minGrid.x, maxGrid.x, minGrid.y, maxGrid.y),
                                          elapsedSeconds);
                        }
                    }
                }
            }
        }

        //If the player has left the bounds of this room, move him into a different room.
        if (!bounds.IsPointInside(players[i]->Pos))
        {
            Level::Instance.PutInRoom(players[i]);
            players.erase(players.begin() + i);
            i -= 1;
        }
    }


    for (unsigned int i = 0; i < otherActors.size(); ++i)
    {
        otherActors[i]->Update(elapsedSeconds);
    }
}
void Room::Render(float elapsed, const RenderInfo& info)
{
    for (unsigned int i = 0; i < players.size(); ++i)
    {
        players[i]->Render(elapsed, info);
    }
    for (unsigned int i = 0; i < otherActors.size(); ++i)
    {
        otherActors[i]->Render(elapsed, info);
    }
}

Room::RaycastResults Room::CastWallRay(Vector3f start, Vector3f dir, Vector3f& hitPos, float& hitT)
{
    assert(dir.XY().LengthSquared() != 0.0f);


    //Move the ray forward till it reaches this room.
    Box2D myBounds = Box2D((float)pos.x, (float)pos.y,
                           ToV2f(baseInfo.RoomGrid.GetDimensions()));
    Vector2f startRoom, endRoom;
    float startT, endT;
    unsigned int nHits = myBounds.CastRay(start.XY(), dir.XY(), startRoom, startT, endRoom, endT);
    switch (nHits)
    {
        case 1:
            endRoom = startRoom;
            endT = startT;
            startT = 0.0f;
            startRoom = start.XY();
            break;

        default:
            assert(nHits == 2);
    }


    //Make sure the ray will actually hit something.
    assert((start.z + (dir.z * startT) >= 0.0f) &&
           (start.z + (dir.z * startT) <= Constants::Instance.CeilingHeight));


    //Get the time for the ray to hit the floor/ceiling.
    Geometryf::PointOnLineAtValueResult<Vector3f> verticalHit(Vector3f(),
                                                              std::numeric_limits<float>::infinity());
    if (dir.z != 0.0f)
    {
        auto floorResult = Geometryf::GetPointOnLineAtValue(start, dir, 2, 0.0f),
             ceilResult = Geometryf::GetPointOnLineAtValue(start, dir, 2,
                                                           Constants::Instance.CeilingHeight);

        assert(floorResult.t < 0.0f || ceilResult.t < 0.0f);
        if (floorResult.t >= 0.0f)
        {
            verticalHit = floorResult;
        }
        else if (ceilResult.t >= 0.0f)
        {
            verticalHit = ceilResult;
        }
    }

    //See if the ceiling/floor will get hit before the end of the room.
    bool willHitVertical = (verticalHit.t < endT);
    if (willHitVertical)
    {
        endT = verticalHit.t;
        endRoom = verticalHit.Point.XY();
    }
    

    //Now that we have the definitive start/end of the ray for this room,
    //    step through every grid spot and see what it hits.
    Vector2f endGrid = startRoom;
    float endGridT = startT;
    while (endGridT < endT)
    {
        const float tIncrement = 0.01f;
        Vector2f startGrid = endGrid + (dir.XY() * tIncrement);
        float startT = endGridT + tIncrement;

        Vector2i gridPos = startGrid.Floored();
        Vector2u relativeGridPos = ToV2u(gridPos - ToV2i(pos));

        if (baseInfo.RoomGrid[relativeGridPos] == BT_WALL)
        {
            hitT = endGridT;
            hitPos = Vector3f(endGrid.x, endGrid.y, start.z + (dir.z * endGridT));
            return RR_WALL;
        }

        //Create a bounding box for this grid element and see where the ray exits it.
        Box2D gridBounds((float)gridPos.x, (float)gridPos.y, Vector2f(1.0f, 1.0f));
        gridBounds.CastRay(start.XY(), dir.XY(), startGrid, startT, endGrid, endT);
    }


    //We didn't hit anything in the room itself.
    if (willHitVertical)
    {
        hitPos = verticalHit.Point;
        hitT = verticalHit.t;
        return (verticalHit.Point.z < 0.1f) ? RR_FLOOR : RR_CEILING;
    }
    else
    {
        hitPos = Vector3f(endRoom.x, endRoom.y, start.z + (dir.z * endT));
        hitT = endT;
        return RR_NOTHING;
    }
}
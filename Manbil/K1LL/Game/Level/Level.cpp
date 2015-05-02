#include "Level.h"

#include "../../../Math/Higher Math/Geometryf.h"
#include "../../Content/Constants.h"


namespace
{
    //Gets whether the given circle touches the given grid spot.
    bool TouchesGridBox(const Circle& circ, Vector2f gridPos)
    {
        Vector2f center(gridPos.x + 0.5f, gridPos.y + 0.5f);

        Vector2f boxToCirc = circ.Pos - center,
                 boxToCircAbs = boxToCirc.Abs();

        //Find out which axis the box is closer along,
        //    and use that to determine which edge will intersect with the circle.
        Vector2f edge1, edge2;
        if (boxToCircAbs.x < boxToCircAbs.y)
        {
            if (boxToCirc.y < 0.0f)
            {
                edge1 = gridPos;
                edge2 = Vector2f(gridPos.x + 1.0f, gridPos.y);
            }
            else
            {
                edge1 = Vector2f(gridPos.x, gridPos.y + 1.0f);
                edge2 = Vector2f(gridPos.x + 1.0f, edge1.y);
            }
        }
        else
        {
            if (boxToCirc.x < 0.0f)
            {
                edge1 = gridPos;
                edge2 = Vector2f(gridPos.x, gridPos.y + 1.0f);
            }
            else
            {
                edge1 = Vector2f(gridPos.x + 1.0f, gridPos.y);
                edge2 = Vector2f(edge1.x, edge1.y + 1.0f);
            }
        }

        return circ.IsPointInside(Geometryf::ClosestToLine(edge1, edge2, circ.Pos, false));
    }
}


Level::Level(const LevelInfo& level, std::string& err)
    : BlockGrid(1, 1), NavGraph(BlockGrid)
{
    LevelInfo::UIntBox bnds = level.GetBounds();

    //Set up the rooms.
    std::vector<RoomNode> tempRooms;
    BlockGrid.Reset(bnds.Max.x - bnds.Min.x, bnds.Max.y - bnds.Min.y, BT_WALL);
    for (unsigned int i = 0; i < level.Rooms.size(); ++i)
    {
        Vector2u min = level.RoomOffsets[i],
                 max = min + level.Rooms[i].RoomGrid.GetDimensions();

        #pragma region Fill in the grid area this room covers

        for (Vector2u counter = min; counter.y <= max.y; ++counter.y)
        {
            for (counter.x = min.x; counter.x <= max.x; ++counter.x)
            {
                BlockTypes& levelType = BlockGrid[counter];
                BlockTypes roomType = level.Rooms[i].RoomGrid[counter - min];

                //If there is already a doorway here, then we either have another doorway or a wall.
                if (levelType == BT_DOORWAY)
                {
                    if (roomType == BT_DOORWAY)
                    {
                        levelType = BT_NONE;
                    }
                    else
                    {
                        assert(roomType == BT_WALL);
                        levelType = BT_WALL;
                    }
                }
                else
                {
                    levelType = roomType;
                }
            }
        }

        #pragma endregion

        #pragma region Turn any unused doorways into walls

        for (Vector2u counter = min; counter.y <= max.y; ++counter.y)
        {
            for (counter.x = min.x; counter.x <= max.x; ++counter.x)
            {
                if (BlockGrid[counter] == BT_DOORWAY)
                {
                    BlockGrid[counter] = BT_WALL;
                }
            }
        }

        #pragma endregion

        #pragma region Find all the rooms that connect to this one

        const RoomInfo& thisRoom = level.Rooms[i];
        Vector2u thisMin = level.RoomOffsets[i],
                 thisMax = thisMin + level.Rooms[i].RoomGrid.GetDimensions() - Vector2u(1, 1);

        RoomNode node(min, max, level.RoomItems[i]);
        assert(RoomGraph.Connections.find(node) == RoomGraph.Connections.end());
        
        //Go through all bordering rooms and find which ones have overlapping doorways.
        std::vector<unsigned int> connections;
        level.GetBorderingRooms(i, connections);
        for (unsigned int j = 0; j < connections.size(); ++j)
        {
            unsigned int indx = connections[j];
            const RoomInfo& otherRoom = level.Rooms[indx];
            Vector2u otherMin = level.RoomOffsets[indx],
                     otherMax = otherMin + level.Rooms[indx].RoomGrid.GetDimensions();

#define SEARCH_EDGE_FOR_DOORWAYS(axisAlongEdge, thisPosX, thisPosY, otherPosX, otherPosY) \
    for (unsigned int i = Mathf::Max(thisMin.axisAlongEdge, otherMin.axisAlongEdge); \
         i < Mathf::Min(thisMax.axisAlongEdge, otherMax.axisAlongEdge); \
         ++i) \
    { \
        Vector2u thisPos(thisPosX, thisPosY), \
                 otherPos(otherPosX, otherPosY); \
        if (thisRoom.RoomGrid[thisPos] == BT_DOORWAY && \
            otherRoom.RoomGrid[otherPos] == BT_DOORWAY) \
        { \
            RoomGraph.Connections[node].push_back(RoomNode(otherMin, otherMax, level.RoomItems[indx])); \
            break; \
        } \
    }
            if (thisMin.x == otherMax.x)
            {
                SEARCH_EDGE_FOR_DOORWAYS(y, thisMin.x, i - thisMin.y, otherMax.x, i - otherMin.y)
            }
            else if (thisMax.x == otherMin.x)
            {
                SEARCH_EDGE_FOR_DOORWAYS(y, thisMax.x, i - thisMin.y, otherMin.x, i - otherMin.y)
            }
            else if (thisMin.y == otherMax.y)
            {
                SEARCH_EDGE_FOR_DOORWAYS(x, i - thisMin.x, thisMin.y, i - otherMin.x, otherMax.y)
            }
            else if (thisMax.y == otherMin.y)
            {
                SEARCH_EDGE_FOR_DOORWAYS(x, i - thisMin.x, thisMax.y, i - otherMin.x, otherMin.y)
            }
        }

        #pragma endregion
    }


    #pragma region Set up important Actors



    #pragma endregion
}

void Level::Update(float elapsed)
{
    Vector2f playerRadius(Constants::Instance.PlayerCollisionRadius,
                          Constants::Instance.PlayerCollisionRadius);
    for (unsigned int i = 0; i < Players.size(); ++i)
    {
        Player& player = *Players[i];
        player.Update(elapsed);

        //Check for any collisions between this player and the walls.
        Vector2f min = player.Pos - playerRadius,
                 max = player.Pos + playerRadius;
        Vector2i minGrid = ToV2i(min),
                 maxGrid = ToV2i(max);
        for (int x = minGrid.x; x <= maxGrid.x; ++x)
        {
            for (int y = minGrid.y; y <= maxGrid.y; ++y)
            {
                if (x < 0 || y < 0 || x >= BlockGrid.GetWidth() || y >= BlockGrid.GetHeight() ||
                    (BlockGrid[ToV2u(Vector2i(x, y))] == BT_WALL &&
                     TouchesGridBox(player.GetCollision2D(), Vector2f((float)x, (float)y))))
                {
                    player.PushOffWall(Box2D((float)x, (float)y, Vector2f(1.0f, 1.0f)), elapsed);
                }
            }
        }
    }

    for (unsigned int i = 0; i < Actors.size(); ++i)
    {
        if (Actors[i]->Update(this, elapsed))
        {
            Actors.erase(Actors.begin() + i);
            i -= 1;
        }
    }
}
void Level::Render(float elapsed, const RenderInfo& info)
{
    for (unsigned int i = 0; i < Players.size(); ++i)
    {
        Players[i]->Render(elapsed, info);
    }
    for (unsigned int i = 0; i < Actors.size(); ++i)
    {
        Actors[i]->Render(this, elapsed, info);
    }
}

Level::RaycastResults Level::CastWallRay(Vector3f start, Vector3f dir, Vector3f& hitPos, float& hitT)
{
    assert(dir.Length() > 0.0f);

    //Handle edge cases.
    if (dir.XY().LengthSquared() == 0.0f)
    {
        if (start.z > Constants::Instance.CeilingHeight)
        {
            hitPos = Vector3f(start.x, start.y, Constants::Instance.CeilingHeight);
            hitT = 0.0f;
            return RR_CEILING;
        }
        else if (start.z < 0.0f)
        {
            hitPos = Vector3f(start.x, start.y, 0.0f);
            hitT = 0.0f;
            return RR_FLOOR;
        }
        else if (dir.z < 0.0f)
        {
            hitPos = Vector3f(start.x, start.y, 0.0f);
            hitT = (0.0f - start.z) / dir.z;
            return RR_FLOOR;
        }
        else
        {
            hitPos = Vector3f(start.x, start.y, Constants::Instance.CeilingHeight);
            hitT = (Constants::Instance.CeilingHeight - start.z) / dir.z;
            return RR_CEILING;
        }
    }
    if (start.x < 0.0f || start.y < 0.0f ||
        start.x > BlockGrid.GetWidth() - 1 || start.y > BlockGrid.GetHeight() - 1)
    {
        hitPos = start;
        hitT = 0.0f;
        return RR_WALL;
    }


    //Get the time/position when the ray hits the floor/ceiling.
    RaycastResults verticalHitType;
    Geometryf::PointOnLineAtValueResult<Vector3f> verticalHit(Vector3f(),
                                                              std::numeric_limits<float>::infinity());
    if (dir.z != 0.0f)
    {
        if (dir.z < 0.0f)
        {
            verticalHit = Geometryf::GetPointOnLineAtValue(start, dir, 2, 0.0f);
            verticalHitType = RR_FLOOR;
        }
        else
        {
            verticalHit = Geometryf::GetPointOnLineAtValue(start, dir, 2,
                                                           Constants::Instance.CeilingHeight);
            verticalHitType = RR_CEILING;
        }
    }

    
    //Step the ray through every grid spot it hits until it leaves the grid or hits the floor/ceiling.

    Vector2f startXY = start.XY(),
             dirXY = dir.XY();

    const float epsilonT = 0.001f;
    Vector2f epsilon = dirXY * epsilonT;

    Vector2f maxPos = ToV2f(BlockGrid.GetDimensions());

    Vector2f hitPos2D = start.XY();
    hitT = 0.0f;

    while (hitT < verticalHit.t &&
           hitPos2D.x > 0.0f && hitPos2D.y > 0.0f &&
           hitPos2D.x < maxPos.x && hitPos2D.y < maxPos.y)
    {
        Box2D blockBnds(floorf(hitPos2D.x), floorf(hitPos2D.y), Vector2f(1.0f, 1.0f));

        //If this grid spot is a wall, we're done.
        if (BlockGrid[ToV2u(hitPos2D)] == BT_WALL)
        {
            blockBnds.CastRay(startXY, dirXY, hitPos2D, hitT);
            hitPos = Vector3f(hitPos2D.x, hitPos2D.y, start.z + (dir.z * hitT));
            return RR_WALL;
        }

        //Otherwise, push the ray to the edge of the box.
        Vector2f dummyV;
        float dummyT;
        unsigned int nHits = blockBnds.CastRay(startXY, dirXY, dummyV, dummyT, hitPos2D, hitT);
        assert(nHits == 2);

        //Edge it forward a bit to make sure it's firmly in the next grid cell.
        hitPos2D += epsilon;
        hitT += epsilonT;
    }


    //We either hit the floor/ceiling or left the bounds of the level.
    if (hitT >= verticalHit.t)
    {
        hitPos = verticalHit.Point;
        hitT = verticalHit.t;
        return verticalHitType;
    }
    else
    {
        //Already calculated the hit pos during the course of the raymarch algorithm.
        hitT -= epsilonT;
        hitPos = Vector3f(hitPos2D.x - epsilon.x, hitPos2D.y - epsilon.y,
                          start.z + (dir.z * hitT));
        return RR_WALL;
    }
}
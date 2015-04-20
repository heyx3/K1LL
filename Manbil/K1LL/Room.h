#pragma once

#include "RoomInfo.h"
#include "Actors/Player.h"
#include "../Math/Shapes/ThreeDShapes.h"


class Room
{
public:

    Room(Vector2u _pos, const RoomInfo& info);


    void Update(float elapsedSeconds);
    void Render(float elapsedSeconds, const RenderInfo& info);


    enum RaycastResults
    {
        RR_NOTHING,
        RR_FLOOR,
        RR_WALL,
        RR_CEILING,
    };

    //PRIORITY: Get rid of the concept of "rooms" after the world is all generated -- move this raycasting stuff (and all the Update() stuff) into the Level class.
    //Figures out what part of the world geometry is hit by the given ray.
    //Assumes the ray WILL pass through the room.
    //Outputs either of the following values into "hitPos"/"hitT":
    //   1. The hit pos in world space, if something was hit.
    //   2. The ray's position just outside of this room after passing through it, if nothing was hit.
    //Assumes the ray is NOT pointing straight up/down.
    RaycastResults CastWallRay(Vector3f start, Vector3f dir, Vector3f& hitPos, float& hitT);


private:
    
    std::vector<PlayerPtr> players;
    std::vector<ActorPtr> otherActors;

    RoomInfo baseInfo;

    //The room's offset from the origin.
    Vector2u pos;

    Box2D bounds;
};

typedef std::shared_ptr<Room> RoomPtr;
#pragma once

#include "RoomInfo.h"
#include "../IO/DataSerialization.h"


//Information about a level. Always assumes that its rooms do not intersect each other.
struct LevelInfo : public ISerializable
{
    //Gets whether the given two boxes (defined by their max and min) touch at all.
    static bool BoxesTouching(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);
    //Gets whether the given two boxes share at least part of an edge.
    //Assumes they don't touch.
    static bool BoxesBorder(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);


    std::vector<RoomInfo> Rooms;
    std::vector<Vector2u> RoomOffsets;

    //The "center" for each team -- has an effect on spawning, objectives, pathing, etc.
    Vector2f TeamOneCenter, TeamTwoCenter;


    //Gets whether the given area is completely devoid of rooms.
    bool IsAreaFree(Vector2u start, Vector2u end) const;

    //Gets the room that the given grid spot is occupied by.
    //Returns 0 if the given spot isn't occupied.
    const RoomInfo* GetRoom(Vector2u worldGridPos) const;

    //Gets all rooms that border the given one.
    //Rooms are specified as indices into this instance's "Rooms" collection.
    void GetBorderingRooms(unsigned int room, std::vector<unsigned int>& outRooms) const;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
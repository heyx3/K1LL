#pragma once

#include "../../IO/DataSerialization.h"

#include "RoomInfo.h"
#include "../Game/Level/RoomsGraph.h"


//Information about a level, defined as a collection of rooms plus some meta-data.
//Note that room can never pass behind the origin.
struct LevelInfo : public ISerializable
{
    //A bounding box using unsigned ints.
    struct UIntBox { Vector2u Min, Max; };

    static const std::string LevelFilesPath;


    //Gets whether the given two boxes (defined by their max and min) touch at all.
    static bool BoxesTouching(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);
    //Gets whether the given two boxes touch inside their edges.
    static bool BoxesTouchingInside(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);
    //Gets whether the given two boxes are touching on an edge, assuming they're not touching inside.
    static bool BoxesBorder(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);


    std::vector<RoomInfo> Rooms;

    //The offsets specify the coordinates of the min (top-left) corner of the room.
    std::vector<Vector2u> RoomOffsets;

    //The type of item each room contains.
    std::vector<ItemTypes> RoomItems;

    //The "center" for each team -- has an effect on spawning, objectives, pathing, etc.
    Vector2f TeamOneCenter, TeamTwoCenter;


    //Gets whether the given area is completely devoid of rooms.
    bool IsAreaFree(Vector2u start, Vector2u end, bool allowRoomEdges) const;

    //Gets the index of the room that the given grid spot is occupied by.
    //Returns "Rooms.size()" if the given spot isn't occupied.
    unsigned int GetRoom(Vector2u worldGridPos) const;

    //Gets all rooms that border the given one.
    //Rooms are specified as indices into this instance's "Rooms" collection.
    void GetBorderingRooms(unsigned int room, std::vector<unsigned int>& outRooms) const;

    //Gets the smallest-possible bounding box covering every room in the level.
    UIntBox GetBounds(void) const;

    //Calculates room connections.
    void GetConnections(RoomsGraph& outGraph) const;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
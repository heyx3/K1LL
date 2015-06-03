#pragma once

#include "../../IO/DataSerialization.h"

#include "RoomInfo.h"
#include "ItemTypes.h"


class RoomsGraph;


//Information about a level, defined as a collection of rooms plus some meta-data.
//Note that room can never pass below the origin.
struct LevelInfo : public ISerializable
{
    //All information about a room in this level.
    struct RoomData : public ISerializable
    {
        //The layout of the room.
        Array2D<BlockTypes> Walls;
        //The position of this room's min corner in the level.
        Vector2u MinCornerPos;
        //The item this room spawns.
        ItemTypes SpawnedItem;
        //The average number of grid spaces needed to traverse from one end of this room to the other.
        float AverageLength;


        RoomData(void) : Walls(1, 1, BT_NONE) { }

        RoomData(const Array2D<BlockTypes>& walls, Vector2u minCornerPos,
                 ItemTypes spawnedItem, float avgLength)
            : Walls(walls.GetWidth(), walls.GetHeight()), MinCornerPos(minCornerPos),
              SpawnedItem(spawnedItem), AverageLength(avgLength)
        {
            walls.MemCopyInto(Walls.GetArray());
        }

        RoomData(const RoomData& cpy) : Walls(cpy.Walls.GetWidth(), cpy.Walls.GetHeight()) { *this = cpy; }
        RoomData& operator=(const RoomData& cpy)
        {
            Walls.Resize(cpy.Walls.GetWidth(), cpy.Walls.GetHeight(), BT_NONE);
            cpy.Walls.MemCopyInto(Walls.GetArray());

            MinCornerPos = cpy.MinCornerPos;
            SpawnedItem = cpy.SpawnedItem;
            AverageLength = cpy.AverageLength;

            return *this;
        }

        RoomData(RoomData&& other)
            : Walls(std::move(other.Walls)), MinCornerPos(other.MinCornerPos),
              SpawnedItem(other.SpawnedItem), AverageLength(other.AverageLength) { }

        RoomData& operator=(RoomData&& other)
        {
            Walls = std::move(other.Walls);
            MinCornerPos = other.MinCornerPos;
            SpawnedItem = other.SpawnedItem;
            AverageLength = other.AverageLength;

            return *this;
        }


        //Gets whether this room has any spawn blocks.
        bool GetHasSpawns(void) const;

        virtual void WriteData(DataWriter* writer) const override;
        virtual void ReadData(DataReader* reader) override;
    };


    //A bounding box using unsigned ints.
    struct UIntBox { Vector2u Min, Max; };


    static const std::string LevelFilesPath;


    //Gets whether the given two boxes (defined by their max and min) touch at all.
    static bool BoxesTouching(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);
    //Gets whether the given two boxes touch inside their edges.
    static bool BoxesTouchingInside(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);
    //Gets whether the given two boxes are touching on an edge, assuming they're not touching inside.
    static bool BoxesBorder(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2);


    std::vector<RoomData> Rooms;

    //The distance from each team's base to the farthest-away point from it on the map.
    float MaxDistToTeam1, MaxDistToTeam2;

    //The room that each team is based in.
    unsigned int Team1Base = 0,
                 Team2Base = 0;


    //Gets whether the given area is completely devoid of rooms.
    bool IsAreaFree(Vector2u start, Vector2u end, bool allowRoomEdges) const;

    //Gets the index of the room that the given grid spot is occupied by.
    //Returns "Rooms.size()" if the given spot isn't occupied.
    unsigned int GetRoom(Vector2u worldGridPos) const;

    //Gets all rooms that border the given one.
    //Rooms are specified as indices into this instance's "Rooms" collection.
    void GetBorderingRooms(unsigned int room, std::vector<unsigned int>& outRooms) const;
    //Gets all rooms that border the given area.
    //Rooms are specified as indices into this instance's "Rooms" collection.
    void GetBorderingRooms(UIntBox area, std::vector<unsigned int>& outRooms) const;

    //Gets the smallest-possible bounding box that covers every room in the level.
    UIntBox GetBounds(void) const;
    //Gets the bounding box of the given room in the level.
    UIntBox GetBounds(unsigned int roomIndex) const;

    //Calculates room connections.
    void GetConnections(RoomsGraph& outGraph) const;

    //Generates a full level grid with this level's rooms.
    //Uses the given bounds as the level grid's bounds. Assumes they're big enough to fit every room.
    void GenerateFullLevel(Array2D<BlockTypes>& outLevel, UIntBox levelBnds) const;
    //Generates a full level grid with this level's rooms.
    //The grid will be as small as possible while still fitting every room.
    void GenerateFullLevel(Array2D<BlockTypes>& outLevel) const { GenerateFullLevel(outLevel, GetBounds()); }


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
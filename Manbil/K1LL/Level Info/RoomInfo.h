#pragma once

#include "../../Math/Lower Math/Array2D.h"
#include "../../Math/Shapes/Boxes.h"
#include "../../IO/DataSerialization.h"


//A room is a rectangular grid of "blocks".
//A block may be anything from empty space to a solid wall.

enum BlockTypes : unsigned char
{
    BT_NONE,
    BT_WALL,
    BT_DOORWAY,
    BT_SPAWN,
};


//Information about a room.
struct RoomInfo : public ISerializable
{
    //The layout of this room.
    Array2D<BlockTypes> RoomGrid;


    RoomInfo(void) : RoomGrid(1, 1, BT_NONE) { }
    RoomInfo(const RoomInfo& cpy);

    inline RoomInfo(RoomInfo&& toMove) : RoomGrid(std::move(toMove.RoomGrid)) { }
    inline RoomInfo& operator=(RoomInfo&& toMove)
    {
        RoomGrid = std::move(toMove.RoomGrid);
        return *this;
    }


    inline RoomInfo& operator=(const RoomInfo& cpy)
    {
        RoomGrid.Resize(cpy.RoomGrid.GetWidth(), cpy.RoomGrid.GetHeight(), BT_NONE);
        cpy.RoomGrid.MemCopyInto(RoomGrid.GetArray());

        return *this;
    }
    
    bool operator==(const RoomInfo& other) const { return RoomGrid.GetArray() == other.RoomGrid.GetArray(); }
    bool operator!=(const RoomInfo& other) const { return !operator==(other); }


    //Gets whether this room has any spawn blocks.
    bool GetRoomHasSpawns(void) const;

    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
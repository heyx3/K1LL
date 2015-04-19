#pragma once

#include "../Math/Lower Math/Array2D.h"
#include "../Math/Shapes/Boxes.h"
#include "../IO/DataSerialization.h"


//A room is a rectangular grid of "blocks".
//A block may be anything from empty space to a solid wall.

enum BlockTypes : unsigned char
{
    BT_NONE,
    BT_WALL,
    BT_PLAYER_SPAWN,
    BT_ITEM_SPAWN,
};


//Information about a room.
struct RoomInfo : public ISerializable
{
    //The categorization of this room. Sub-categories can be specified like a file hierarchy --
    //    e.x. "Small/Open".
    std::string Category;

    //How complex, from 0 to 1, is this room in terms of navigation?
    float NavigationDifficulty;

    //The layout of this room.
    Array2D<BlockTypes> RoomGrid;


    inline RoomInfo(RoomInfo&& toMove)
        : RoomGrid(std::move(toMove.RoomGrid))
    {
        Category = std::move(toMove.Category);
        NavigationDifficulty = toMove.NavigationDifficulty;
    }
    inline RoomInfo& operator=(RoomInfo&& toMove)
    {
        Category = std::move(toMove.Category);
        NavigationDifficulty = toMove.NavigationDifficulty;
        RoomGrid = std::move(toMove.RoomGrid);

        return *this;
    }

    
    bool operator==(const RoomInfo& other) const { return RoomGrid.GetArray() == other.RoomGrid.GetArray(); }
    bool operator!=(const RoomInfo& other) const { return !operator==(other); }


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
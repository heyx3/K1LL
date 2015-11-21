#include "RoomInfo.h"

#include "../../IO/DataSerialization.h"


RoomInfo::RoomInfo(const RoomInfo& cpy)
    : RoomGrid(cpy.RoomGrid.GetWidth(), cpy.RoomGrid.GetHeight())
{
    cpy.RoomGrid.MemCopyInto(RoomGrid.GetArray());
}

bool RoomInfo::GetRoomHasSpawns(void) const
{
    for (unsigned int y = 0; y < RoomGrid.GetHeight(); ++y)
    {
        for (unsigned int x = 0; x < RoomGrid.GetWidth(); ++x)
        {
            if (RoomGrid[Vector2u(x, y)] == BT_SPAWN)
            {
                return true;
            }
        }
    }

    return false;
}

void RoomInfo::WriteData(DataWriter* writer) const
{
    writer->WriteUInt(RoomGrid.GetWidth(), "Grid width");
    writer->WriteUInt(RoomGrid.GetHeight(), "Grid height");

    for (unsigned int x = 0; x < RoomGrid.GetWidth(); ++x)
    {
        for (unsigned int y = 0; y < RoomGrid.GetHeight(); ++y)
        {
            writer->WriteByte(RoomGrid[Vector2u(x, y)],
                              std::to_string(x) + "x" + std::to_string(y));
        }
    }
}
void RoomInfo::ReadData(DataReader* reader)
{
    Vector2u gridSize;
    reader->ReadUInt(gridSize.x);
    reader->ReadUInt(gridSize.y);
    RoomGrid.Resize(gridSize.x, gridSize.y, BT_NONE);

    for (unsigned int x = 0; x < gridSize.x; ++x)
    {
        for (unsigned int y = 0; y < gridSize.y; ++y)
        {
            unsigned char b;
            reader->ReadByte(b);
            RoomGrid[Vector2u(x, y)] = (BlockTypes)b;
        }
    }
}
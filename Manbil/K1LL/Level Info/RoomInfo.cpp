#include "RoomInfo.h"

#include "../../IO/Serialization.h"


RoomInfo::RoomInfo(const RoomInfo& cpy)
    : NavigationDifficulty(cpy.NavigationDifficulty),
      RoomGrid(cpy.RoomGrid.GetWidth(), cpy.RoomGrid.GetHeight())
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
    writer->WriteFloat(NavigationDifficulty, "Navigation difficulty (0-1)");

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
    reader->ReadFloat(NavigationDifficulty);

    Vector2u gridSize;
    reader->ReadUInt(gridSize.x);
    reader->ReadUInt(gridSize.y);
    RoomGrid.Resize(gridSize.x, gridSize.y, BT_NONE);

    for (unsigned int x = 0; x < gridSize.x; ++x)
    {
        for (unsigned int y = 0; y < gridSize.y; ++y)
        {
            reader->ReadByte((unsigned char&)RoomGrid[Vector2u(x, y)]);
        }
    }
}
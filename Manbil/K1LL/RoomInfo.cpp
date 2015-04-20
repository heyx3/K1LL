#include "RoomInfo.h"

#include "../IO/Serialization.h"


void RoomInfo::WriteData(DataWriter* writer) const
{
    writer->WriteString(Category, "Category");
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

    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned int i, void* pDat)
                            {
                                const Vector2u& v = *(const Vector2u*)toWrite;
                                writer->WriteDataStructure(Vector2u_Writable(v), "Node");
                            },
                            "Nav nodes", sizeof(Vector2u), NavNodes.data(), NavNodes.size());
}
void RoomInfo::ReadData(DataReader* reader)
{
    reader->ReadString(Category);
    reader->ReadFloat(NavigationDifficulty);

    Vector2u gridSize;
    reader->ReadUInt(gridSize.x);
    reader->ReadUInt(gridSize.y);
    RoomGrid.Resize(gridSize.x, gridSize.y);

    for (unsigned int x = 0; x < gridSize.x; ++x)
    {
        for (unsigned int y = 0; y < gridSize.y; ++y)
        {
            reader->ReadByte((unsigned char&)RoomGrid[Vector2u(x, y)]);
        }
    }

    reader->ReadCollection([](DataReader* reader, void* collection, unsigned int i, void* pData)
                           {
                               auto& coll = *(std::vector<Vector2u>*)collection;
                               reader->ReadDataStructure(Vector2u_Readable(coll[i]));
                           },
                           [](void* collection, unsigned int newSize)
                           {
                               ((std::vector<Vector2u>*)collection)->resize(newSize);
                           },
                           &NavNodes);
}
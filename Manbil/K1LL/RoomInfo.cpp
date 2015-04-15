#include "RoomInfo.h"

#include "../IO/Serialization.h"


void RoomInfo::WriteData(DataWriter* writer) const
{
    writer->WriteDataStructure(Vector2u_Writable(GridSize), "Grid size");
    writer->WriteFloat(NavigationDifficulty, "Navigation difficulty (0-1)");

    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned elIndex, void* pData)
                            {
                                const Vector2u& v2u = (*(Vector2u*)toWrite);
                                writer->WriteDataStructure(Vector2u_Writable(v2u), "Pos");
                            },
                            "Connecting grid positions", sizeof(Vector2u),
                            ConnectingGridPoses.data(), ConnectingGridPoses.size());
    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned elIndex, void* pData)
                            {
                                const Vector2f& v2f = (*(Vector2f*)toWrite);
                                writer->WriteDataStructure(Vector2f_Writable(v2f), "Pos");
                            },
                            "Item spawn positions", sizeof(Vector2f),
                            ItemSpawnPoses.data(), ItemSpawnPoses.size());
    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned elIndex, void* pData)
                            {
                                const Box2D& region = (*(Box2D*)toWrite);
                                writer->WriteDataStructure(Vector2f_Writable(region.GetCenter()),
                                                           "Center");
                                writer->WriteDataStructure(Vector2f_Writable(region.GetDimensions()),
                                                           "Size");
                            },
                            "Player spawn regions", sizeof(Box2D),
                            PlayerSpawnRegions.data(), PlayerSpawnRegions.size());
    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned elIndex, void* pData)
                            {
                                const LineSegment& line = (*(LineSegment*)toWrite);
                                writer->WriteDataStructure(Vector2f_Writable(line.Start), "Start");
                                writer->WriteDataStructure(Vector2f_Writable(line.End), "End");
                            },
                            "Walls", sizeof(LineSegment), Walls.data(), Walls.size());
}
void RoomInfo::ReadData(DataReader* reader)
{
    reader->ReadDataStructure(Vector2u_Readable(GridSize));
    reader->ReadFloat(NavigationDifficulty);

    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int elIndex, void* pData)
                           {
                               auto& coll = *(std::vector<Vector2u>*)pCollection;
                               reader->ReadDataStructure(Vector2u_Readable(coll[elIndex]));
                           },
                           [](void* pCollection, unsigned int nElements)
                           {
                               auto& coll = *(std::vector<Vector2u>*)pCollection;
                               coll.resize(nElements);
                           },
                           &ConnectingGridPoses);
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int elIndex, void* pData)
                           {
                               auto& coll = *(std::vector<Vector2f>*)pCollection;
                               reader->ReadDataStructure(Vector2f_Readable(coll[elIndex]));
                           },
                           [](void* pCollection, unsigned int nElements)
                           {
                               auto& coll = *(std::vector<Vector2f>*)pCollection;
                               coll.resize(nElements);
                           },
                           &ItemSpawnPoses);
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int elIndex, void* pData)
                           {
                               auto& coll = *(std::vector<Box2D>*)pCollection;
                               Vector2f center, size;
                               reader->ReadDataStructure(Vector2f_Readable(center));
                               reader->ReadDataStructure(Vector2f_Readable(size));
                               coll[elIndex] = Box2D(center, size);
                           },
                           [](void* pCollection, unsigned int nElements)
                           {
                               auto& coll = *(std::vector<Box2D>*)pCollection;
                               coll.resize(nElements);
                           },
                           &PlayerSpawnRegions);
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int elIndex, void* pData)
                           {
                               auto& coll = *(std::vector<LineSegment>*)pCollection;
                               Vector2f start, end;
                               reader->ReadDataStructure(Vector2f_Readable(start));
                               reader->ReadDataStructure(Vector2f_Readable(end));
                               coll[elIndex] = LineSegment(start, end);
                           },
                           [](void* pCollection, unsigned int nElements)
                           {
                               auto& coll = *(std::vector<Box2D>*)pCollection;
                               coll.resize(nElements);
                           },
                           &Walls);
}
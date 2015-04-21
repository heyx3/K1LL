#include "RoomCollection.h"


void RoomCollection::WriteData(DataWriter* writer) const
{
    writer->WriteCollection(
                [](DataWriter* writer, const void* toWrite, unsigned int i, void* pData)
                {
                    writer->WriteDataStructure(*(const RoomInfo*)toWrite, "Room");
                },
                "Rooms", sizeof(RoomInfo), Rooms.data(), Rooms.size());
}
void RoomCollection::ReadData(DataReader* reader)
{
    reader->ReadCollection(
                [](DataReader* reader, void* pCollection, unsigned int i, void* pData)
                {
                    auto& coll = *(std::vector<RoomInfo>*)pCollection;
                    reader->ReadDataStructure(coll[i]);
                },
                [](void* pCollection, unsigned int newSize)
                {
                    auto& coll = *(std::vector<RoomInfo>*)pCollection;
                    coll.resize(newSize);
                },
                &Rooms);
}
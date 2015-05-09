#include "LevelInfo.h"

#include "../../IO/Serialization.h"


const std::string LevelInfo::LevelFilesPath = "Content/Levels/";


inline bool IsInBox(Vector2u boxMin, Vector2u boxMax, Vector2u pos)
{
    return (pos.x > boxMin.x && pos.x < boxMax.x && pos.y > boxMin.y && pos.y < boxMax.y);
}
inline bool IsTouchingBox(Vector2u boxMin, Vector2u boxMax, Vector2u pos)
{
    return (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y);
}

bool LevelInfo::BoxesTouchingInside(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2)
{
    return IsInBox(min1, max1, min2) || IsInBox(min1, max1, max2) ||
           IsInBox(min1, max1, Vector2u(min2.x, max2.y)) ||
           IsInBox(min1, max1, Vector2u(min2.x, max2.y)) ||

           IsInBox(min2, max2, min1) || IsInBox(min2, max2, max1) ||
           IsInBox(min2, max2, Vector2u(min1.x, max1.y)) ||
           IsInBox(min2, max2, Vector2u(min1.x, max1.y));
}
bool LevelInfo::BoxesTouching(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2)
{
    return IsTouchingBox(min1, max1, min2) || IsInBox(min1, max1, max2) ||
           IsTouchingBox(min1, max1, Vector2u(min2.x, max2.y)) ||
           IsTouchingBox(min1, max1, Vector2u(min2.x, max2.y)) ||

           IsTouchingBox(min2, max2, min1) || IsInBox(min2, max2, max1) ||
           IsTouchingBox(min2, max2, Vector2u(min1.x, max1.y)) ||
           IsTouchingBox(min2, max2, Vector2u(min1.x, max1.y));
}
bool LevelInfo::BoxesBorder(Vector2u min1, Vector2u max1, Vector2u min2, Vector2u max2)
{
    return (min1.x == max2.x || max1.x == min2.x ||
            min1.y == max2.y || max1.y == min2.y);
}

bool LevelInfo::IsAreaFree(Vector2u start, Vector2u end, bool allowEdges) const
{
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        Vector2u min = RoomOffsets[i],
                 max = min + Rooms[i].RoomGrid.GetDimensions();

        if (BoxesTouchingInside(start, end, min, max) ||
            (!allowEdges && BoxesBorder(start, end, min, max)))
        {
            return false;
        }
    }

    return true;
}

const RoomInfo* LevelInfo::GetRoom(Vector2u worldGridPos) const
{
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        Vector2u min = RoomOffsets[i],
                 max = min + Rooms[i].RoomGrid.GetDimensions();
        if (worldGridPos.x >= min.x && worldGridPos.x <= max.x &&
            worldGridPos.y >= min.y && worldGridPos.y <= max.y)
        {
            return &Rooms[i];
        }
    }

    return 0;
}

void LevelInfo::GetBorderingRooms(unsigned int room, std::vector<unsigned int>& outRooms) const
{
    Vector2u myMin = RoomOffsets[room],
             myMax = myMin + Rooms[room].RoomGrid.GetDimensions();

    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        if (i != room)
        {
            Vector2u hisMin = RoomOffsets[i],
                     hisMax = hisMin + Rooms[i].RoomGrid.GetDimensions();

            if (BoxesBorder(myMin, myMax, hisMin, hisMax))
            {
                outRooms.push_back(i);
            }
        }
    }
}

LevelInfo::UIntBox LevelInfo::GetBounds(void) const
{
    UIntBox box;

    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        Vector2u min = RoomOffsets[i],
                 max = min + Rooms[i].RoomGrid.GetDimensions();

        box.Min.x = Mathf::Min(box.Min.x, min.x);
        box.Min.y = Mathf::Min(box.Min.y, min.y);
        box.Max.x = Mathf::Max(box.Max.x, max.x);
        box.Max.y = Mathf::Max(box.Max.y, max.y);
    }

    return box;
}

void LevelInfo::GetConnections(RoomsGraph& outGraph) const
{
    outGraph.Connections.clear();

    UIntBox bnds = GetBounds();

    std::vector<RoomNode> tempRooms;
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        Vector2u min = RoomOffsets[i],
                 max = min + Rooms[i].RoomGrid.GetDimensions();

        const RoomInfo& thisRoom = Rooms[i];
        Vector2u thisMin = RoomOffsets[i],
                 thisMax = thisMin + Rooms[i].RoomGrid.GetDimensions() - Vector2u(1, 1);

        RoomNode node(min, max, RoomItems[i], Rooms[i].NavigationDifficulty, i);
        assert(outGraph.Connections.find(node) == outGraph.Connections.end());
        
        //Go through all bordering rooms and find which ones have overlapping doorways.
        std::vector<unsigned int> connections;
        GetBorderingRooms(i, connections);
        for (unsigned int j = 0; j < connections.size(); ++j)
        {
            unsigned int indx = connections[j];
            const RoomInfo& otherRoom = Rooms[indx];
            Vector2u otherMin = RoomOffsets[indx],
                     otherMax = otherMin + Rooms[indx].RoomGrid.GetDimensions();

#define SEARCH_EDGE_FOR_DOORWAYS(axisAlongEdge, thisPosX, thisPosY, otherPosX, otherPosY) \
    for (unsigned int i = Mathf::Max(thisMin.axisAlongEdge, otherMin.axisAlongEdge); \
         i < Mathf::Min(thisMax.axisAlongEdge, otherMax.axisAlongEdge); \
         ++i) \
    { \
        Vector2u thisPos(thisPosX, thisPosY), \
                 otherPos(otherPosX, otherPosY); \
        if (thisRoom.RoomGrid[thisPos] == BT_DOORWAY && \
            otherRoom.RoomGrid[otherPos] == BT_DOORWAY) \
        { \
            outGraph.Connections[node].push_back(RoomNode(otherMin, otherMax, \
                                                          RoomItems[indx], \
                                                          Rooms[indx].NavigationDifficulty, indx)); \
            break; \
        } \
    }
            if (thisMin.x == otherMax.x)
            {
                SEARCH_EDGE_FOR_DOORWAYS(y, thisMin.x, i - thisMin.y, otherMax.x, i - otherMin.y)
            }
            else if (thisMax.x == otherMin.x)
            {
                SEARCH_EDGE_FOR_DOORWAYS(y, thisMax.x, i - thisMin.y, otherMin.x, i - otherMin.y)
            }
            else if (thisMin.y == otherMax.y)
            {
                SEARCH_EDGE_FOR_DOORWAYS(x, i - thisMin.x, thisMin.y, i - otherMin.x, otherMax.y)
            }
            else if (thisMax.y == otherMin.y)
            {
                SEARCH_EDGE_FOR_DOORWAYS(x, i - thisMin.x, thisMax.y, i - otherMin.x, otherMin.y)
            }
        }
    }
}

void LevelInfo::WriteData(DataWriter* writer) const
{
    writer->WriteDataStructure(Vector2f_Writable(TeamOneCenter), "Team one's center");
    writer->WriteDataStructure(Vector2f_Writable(TeamTwoCenter), "Team two's center");

    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned int i, void* p)
                            {
                                writer->WriteDataStructure(*(const RoomInfo*)toWrite, "Room");
                            }, "Rooms", sizeof(RoomInfo), Rooms.data(), Rooms.size());
    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned int i, void* p)
                            {
                                writer->WriteDataStructure(Vector2u_Writable(*(const Vector2u*)toWrite),
                                                           "Room offset");
                            }, "Room offsets", sizeof(Vector2u), RoomOffsets.data(), RoomOffsets.size());
    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned int i, void* p)
                            {
                                writer->WriteInt((int)*(ItemTypes*)toWrite, "Item");
                            }, "Room items", sizeof(ItemTypes), RoomItems.data(), RoomItems.size());
}
void LevelInfo::ReadData(DataReader* reader)
{
    reader->ReadDataStructure(Vector2f_Readable(TeamOneCenter));
    reader->ReadDataStructure(Vector2f_Readable(TeamTwoCenter));
    
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int i, void* p)
                           {
                               std::vector<RoomInfo>& infos = *(std::vector<RoomInfo>*)pCollection;
                               reader->ReadDataStructure(infos[i]);
                           },
                           [](void* pCollection, unsigned int newSize)
                           {
                               std::vector<RoomInfo>& infos = *(std::vector<RoomInfo>*)pCollection;
                               infos.resize(newSize);
                           },
                           &Rooms);
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int i, void* p)
                           {
                               std::vector<Vector2u>& offs = *(std::vector<Vector2u>*)pCollection;
                               reader->ReadDataStructure(Vector2u_Readable(offs[i]));
                           },
                           [](void* pCollection, unsigned int newSize)
                           {
                               std::vector<Vector2u>& infos = *(std::vector<Vector2u>*)pCollection;
                               infos.resize(newSize);
                           },
                           &RoomOffsets);
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int i, void* p)
                           {
                               std::vector<ItemTypes>& types = *(std::vector<ItemTypes>*)pCollection;
                               int read;
                               reader->ReadInt(read);
                               types[i] = (ItemTypes)read;
                           },
                           [](void* pCollection, unsigned int newSize)
                           {
                               std::vector<ItemTypes>& types = *(std::vector<ItemTypes>*)pCollection;
                               types.resize(newSize);
                           },
                           &RoomItems);
}
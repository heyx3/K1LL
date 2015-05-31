#include "LevelInfo.h"

#include "../../IO/Serialization.h"



const std::string LevelInfo::LevelFilesPath = "Content/Levels/";


bool LevelInfo::RoomData::GetHasSpawns(void) const
{
    for (unsigned int y = 0; y < Walls.GetHeight(); ++y)
        for (unsigned int x = 0; x < Walls.GetWidth(); ++x)
            if (Walls[Vector2u(x, y)] == BT_SPAWN)
                return true;
    return false;
}

void LevelInfo::RoomData::WriteData(DataWriter* writer) const
{
    writer->WriteUInt(Walls.GetWidth(), "Width");
    writer->WriteUInt(Walls.GetHeight(), "Height");
    
    for (unsigned int y = 0; y < Walls.GetHeight(); ++y)
    {
        for (unsigned int x = 0; x < Walls.GetWidth(); ++x)
        {
            writer->WriteByte((unsigned char)Walls[Vector2u(x, y)], "Element");
        }
    }

    writer->WriteDataStructure(Vector2u_Writable(MinCornerPos), "Min corner pos");
    writer->WriteByte((unsigned char)SpawnedItem, "Spawned item");
    writer->WriteFloat(NavDifficultyHorz, "Nav difficulty horizontal");
    writer->WriteFloat(NavDifficultyVert, "Nav difficulty vertical");
}
void LevelInfo::RoomData::ReadData(DataReader* reader)
{
    Vector2u gridSize;
    reader->ReadUInt(gridSize.x);
    reader->ReadUInt(gridSize.y);

    Walls.Resize(gridSize.x, gridSize.y, BT_NONE);
    
    for (unsigned int y = 0; y < Walls.GetHeight(); ++y)
    {
        for (unsigned int x = 0; x < Walls.GetWidth(); ++x)
        {
            unsigned char b;
            reader->ReadByte(b);
            Walls[Vector2u(x, y)] = (BlockTypes)b;
        }
    }

    reader->ReadDataStructure(Vector2u_Readable(MinCornerPos));

    unsigned char b;
    reader->ReadByte(b);
    SpawnedItem = (ItemTypes)b;
    
    reader->ReadFloat(NavDifficultyHorz);
    reader->ReadFloat(NavDifficultyVert);
}


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
        UIntBox bnds = GetBounds(i);

        if (allowEdges)
        {
            if (start.x < bnds.Max.x && end.x > bnds.Min.x &&
                start.y < bnds.Max.y && end.y > bnds.Min.y)
            {
                return false;
            }
        }
        else
        {
            if (start.x <= bnds.Max.x && end.x >= bnds.Min.x &&
                start.y <= bnds.Max.y && end.y >= bnds.Min.y)
            {
                return false;
            }
        }
    }

    return true;
}

unsigned int LevelInfo::GetRoom(Vector2u worldGridPos) const
{
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        UIntBox bnds = GetBounds(i);
        if (worldGridPos.x >= bnds.Min.x && worldGridPos.x <= bnds.Max.x &&
            worldGridPos.y >= bnds.Min.y && worldGridPos.y <= bnds.Max.y)
        {
            return i;
        }
    }

    return Rooms.size();
}


namespace
{
    void GetBordering(LevelInfo::UIntBox bnds, std::vector<unsigned int>& outRooms,
                      const LevelInfo* info, unsigned int skipIndex)
    {
        for (unsigned int i = 0; i < info->Rooms.size(); ++i)
        {
            if (i != skipIndex)
            {
                LevelInfo::UIntBox hisBnds = info->GetBounds(i);

                if (LevelInfo::BoxesBorder(bnds.Min, bnds.Max, bnds.Min, bnds.Max))
                {
                    outRooms.push_back(i);
                }
            }
        }
    }
}
void LevelInfo::GetBorderingRooms(unsigned int room, std::vector<unsigned int>& outRooms) const
{
    GetBordering(GetBounds(room), outRooms, this, room);
}

void LevelInfo::GetBorderingRooms(UIntBox myBnds, std::vector<unsigned int>& outRooms) const
{
    GetBordering(myBnds, outRooms, this, Rooms.size());
}

LevelInfo::UIntBox LevelInfo::GetBounds(void) const
{
    UIntBox box;
    box.Min = Vector2u(999999999, 999999999);
    box.Max = Vector2u();

    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        UIntBox bnds = GetBounds(i);
        box.Min.x = Mathf::Min(box.Min.x, bnds.Min.x);
        box.Min.y = Mathf::Min(box.Min.y, bnds.Min.y);
        box.Max.x = Mathf::Max(box.Max.x, bnds.Max.x);
        box.Max.y = Mathf::Max(box.Max.y, bnds.Max.y);
    }

    return box;
}
LevelInfo::UIntBox LevelInfo::GetBounds(unsigned int roomIndex) const
{
    UIntBox box;

    assert(roomIndex < Rooms.size());
    assert(Rooms[roomIndex].Walls.GetWidth() > 0 && Rooms[roomIndex].Walls.GetHeight() > 0);

    box.Min = Rooms[roomIndex].MinCornerPos;
    box.Max = box.Min + Rooms[roomIndex].Walls.GetDimensions() - Vector2u(1, 1);
    return box;
}

void LevelInfo::GetConnections(RoomsGraph& outGraph) const
{
    outGraph.Connections.clear();
    outGraph.AreConnectionsHorizontal.clear();

    UIntBox bnds = GetBounds();

    std::vector<RoomNode> tempRooms;
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        const RoomData& thisRoom = Rooms[i];
        UIntBox thisBnds = GetBounds(i);

        RoomNode node(thisBnds .Min, thisBnds .Max, thisRoom.SpawnedItem,
                      thisRoom.NavDifficultyHorz, thisRoom.NavDifficultyVert, i);
        assert(outGraph.Connections.find(node) == outGraph.Connections.end());
        
        //Go through all bordering rooms and find which ones have overlapping doorways.
        std::vector<unsigned int> connections;
        GetBorderingRooms(i, connections);
        for (unsigned int j = 0; j < connections.size(); ++j)
        {
            unsigned int indx = connections[j];

            const RoomData& otherRoom = Rooms[indx];
            UIntBox otherBnds = GetBounds(indx);

#define SEARCH_EDGE_FOR_DOORWAYS(axisAlongEdge, thisPosX, thisPosY, otherPosX, otherPosY, isHorz) \
    for (unsigned int k = Mathf::Max(thisBnds.Min.axisAlongEdge, otherBnds.Min.axisAlongEdge); \
         k < Mathf::Min(thisBnds.Max.axisAlongEdge, otherBnds.Max.axisAlongEdge); \
         ++k) \
    { \
        Vector2u thisPos(thisPosX, thisPosY), \
                 otherPos(otherPosX, otherPosY); \
        if (thisRoom.Walls[thisPos] == BT_DOORWAY && \
            otherRoom.Walls[otherPos] == BT_DOORWAY) \
        { \
            RoomNode connection(otherBnds.Min, otherBnds.Max, otherRoom.SpawnedItem, \
                                otherRoom.NavDifficultyHorz, otherRoom.NavDifficultyVert, indx); \
            outGraph.Connections[node].push_back(connection); \
            outGraph.AreConnectionsHorizontal[node][connection] = isHorz; \
            break; \
        } \
    }
            if (thisBnds.Min.x == otherBnds.Max.x)
            {
                SEARCH_EDGE_FOR_DOORWAYS(y, thisBnds.Min.x, k - thisBnds.Min.y,
                                         otherBnds.Max.x, k - otherBnds.Min.y,
                                         true)
            }
            else if (thisBnds.Max.x == otherBnds.Min.x)
            {
                SEARCH_EDGE_FOR_DOORWAYS(y, thisBnds.Max.x, k - thisBnds.Min.y,
                                         otherBnds.Min.x, k - otherBnds.Min.y,
                                         true)
            }
            else if (thisBnds.Min.y == otherBnds.Max.y)
            {
                SEARCH_EDGE_FOR_DOORWAYS(x, k - thisBnds.Min.x, thisBnds.Min.y,
                                         k - otherBnds.Min.x, otherBnds.Max.y,
                                         false)
            }
            else if (thisBnds.Max.y == otherBnds.Min.y)
            {
                SEARCH_EDGE_FOR_DOORWAYS(x, k - thisBnds.Min.x, thisBnds.Max.y,
                                         k - otherBnds.Min.x, otherBnds.Min.y,
                                         false)
            }
        }
    }
}

void LevelInfo::GenerateFullLevel(Array2D<BlockTypes>& outLevel, UIntBox bnds) const
{
    outLevel.Reset(bnds.Max.x - bnds.Min.x, bnds.Max.y - bnds.Min.y, BT_WALL);
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        UIntBox roomBnds = GetBounds(i);

        //Fill in the grid area that this room covers.
        for (Vector2u counter = roomBnds.Min; counter.y <= roomBnds.Max.y; ++counter.y)
        {
            for (counter.x = roomBnds.Min.x; counter.x <= roomBnds.Max.x; ++counter.x)
            {
                BlockTypes& levelSpaceType = outLevel[counter - bnds.Min];
                BlockTypes roomSpaceType = Rooms[i].Walls[counter - roomBnds.Min];

                //If there is already a doorway here, then we either have another doorway or a wall.
                if (levelSpaceType == BT_DOORWAY)
                {
                    if (roomSpaceType == BT_DOORWAY)
                    {
                        levelSpaceType = BT_NONE;
                    }
                    else
                    {
                        assert(roomSpaceType == BT_WALL);
                        levelSpaceType = BT_WALL;
                    }
                }
                //Otherwise, just write this block into the level grid.
                else
                {
                    levelSpaceType = roomSpaceType;
                }
            }
        }
    }

    //Turn any unused doorways into walls.
    for (Vector2u counter = Vector2u(); counter.y <= outLevel.GetHeight(); ++counter.y)
    {
        for (counter.x = 0; counter.x <= outLevel.GetWidth(); ++counter.x)
        {
            if (outLevel[counter] == BT_DOORWAY)
            {
                outLevel[counter] = BT_WALL;
            }
        }
    }
}

void LevelInfo::WriteData(DataWriter* writer) const
{
    writer->WriteUInt(Team1Base, "Team one's room");
    writer->WriteUInt(Team2Base, "Team two's room");
    
    writer->WriteFloat(MaxDistToTeam1, "Max dist to team 1's room");
    writer->WriteFloat(MaxDistToTeam2, "Max dist to team 2's room");

    writer->WriteCollection([](DataWriter* writer, const void* toWrite, unsigned int i, void* p)
                            {
                                writer->WriteDataStructure(*(const RoomData*)toWrite, "Room");
                            }, "Rooms", sizeof(RoomData), Rooms.data(), Rooms.size());
}
void LevelInfo::ReadData(DataReader* reader)
{
    reader->ReadUInt(Team1Base);
    reader->ReadUInt(Team2Base);
    
    reader->ReadFloat(MaxDistToTeam1);
    reader->ReadFloat(MaxDistToTeam2);
    
    reader->ReadCollection([](DataReader* reader, void* pCollection, unsigned int i, void* p)
                           {
                               std::vector<RoomData>& infos = *(std::vector<RoomData>*)pCollection;
                               reader->ReadDataStructure(infos[i]);
                           },
                           [](void* pCollection, unsigned int newSize)
                           {
                               std::vector<RoomData>& infos = *(std::vector<RoomData>*)pCollection;
                               infos.resize(newSize);
                           },
                           &Rooms);
}
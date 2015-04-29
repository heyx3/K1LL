#include "Level.h"


Level::Level(const LevelInfo& level, std::string& err)
    : BlockGrid(1, 1), NavGraph(BlockGrid)
{
    LevelInfo::UIntBox bnds = level.GetBounds();

    //Set up the rooms.
    std::vector<RoomNode> tempRooms;
    BlockGrid.Reset(bnds.Max.x - bnds.Min.x, bnds.Max.y - bnds.Min.y, BT_WALL);
    for (unsigned int i = 0; i < level.Rooms.size(); ++i)
    {
        Vector2u min = level.RoomOffsets[i],
                 max = min + level.Rooms[i].RoomGrid.GetDimensions();

        //Fill in the grid.
        for (Vector2u counter = min; counter.y <= max.y; ++counter.y)
        {
            for (counter.x = min.x; counter.x <= max.x; ++counter.x)
            {
                BlockGrid[counter] = level.Rooms[i].RoomGrid[counter - min];
            }
        }


        RoomNode node(min, max, level.RoomItems[i]);
        
        std::vector<unsigned int> connections;
        level.GetBorderingRooms(i, connections);
        assert(RoomGraph.Connections.find(node) == RoomGraph.Connections.end());
        for (unsigned int j = 0; j < connections.size(); ++j)
        {
            const RoomInfo& inf = level.Rooms[i];

            //RoomGraph.Connections[node].push_back(level.Rooms[connections[j]]);
        }
    }
}

void Level::Update(float elapsed)
{
    //for (unsigned int i = 0; i < Rooms.size(); ++i)
   // {
    //    Rooms[i]->Update(elapsed);
   // }
}
void Level::Render(float elapsed, const RenderInfo& info)
{

}
#include "GUILevelPathing.h"

#include "../../Content/MenuContent.h"

#include "LevelEditor.h"
#include "../../Game/Level/LevelGraph.h"


#define MC MenuContent::Instance


namespace
{
    //Coloring constants.
    Vector4f COLOR_Team1(1.0f, 0.0f, 0.0f, 0.85f),
             COLOR_Team2(0.0f, 0.0f, 1.0f, 0.85f);
}


GUILevelPathing::GUILevelPathing(LevelEditor& _editor)
    : editor(_editor), pather(&graph), levelGrid(1, 1, BT_NONE),
      GUITexture(MC.StaticColorGUINoTexParams, 0, MC.StaticColorGUIMatNoTex)
{

}

void GUILevelPathing::OnRoomsChanged(void)
{
    LevelInfo::UIntBox bnds = editor.LevelData.GetBounds();
    bnds.Min = Vector2u();
    editor.LevelData.GenerateFullLevel(levelGrid, bnds);

    roomsToPath = editor.LevelData.Rooms.size();
    roomsToNav = roomsToPath;

    nStepsFromRoomToTeamBases.resize(editor.LevelData.Rooms.size(), std::array<unsigned int, 2>());
    roomNormalizedDistsToTeamBases.resize(editor.LevelData.Rooms.size(), std::array<float, 2>());

    editor.LevelData.GetConnections(graph);
}

void GUILevelPathing::CustomUpdate(float elapsedTime, Vector2f relativeMouse)
{
    LevelInfo& lvl = editor.LevelData;

    //If rooms need to have their pathing info updated, pick one of them and update it.
    if (roomsToNav > 0)
    {
        #pragma region Calculate "Average Length" for a room

        assert(roomsToNav <= lvl.Rooms.size());

        //Figure out the average path length to pass through the room.

        LevelInfo::RoomData& room = lvl.Rooms[roomsToNav - 1];
        LevelInfo::UIntBox rmBnds = lvl.GetBounds(roomsToNav - 1);

        Array2D<BlockTypes> tempRoom(room.Walls.GetWidth(), room.Walls.GetHeight());
        tempRoom.Fill(levelGrid, -ToV2i(room.MinCornerPos));

        //Get a list of all open end-points of the block.
        std::vector<Vector2u> openSpaces;
        openSpaces.reserve((2 * tempRoom.GetWidth()) + (2 * tempRoom.GetHeight()));
        for (unsigned int x = 0; x < tempRoom.GetWidth(); ++x)
        {
            if (tempRoom[Vector2u(x, 0)] == BT_NONE)
            {
                openSpaces.push_back(Vector2u(x, 0));
            }
            if (tempRoom[Vector2u(x, tempRoom.GetHeight() - 1)] == BT_NONE)
            {
                openSpaces.push_back(Vector2u(x, tempRoom.GetHeight() - 1));
            }
        }
        for (unsigned int y = 0; y < tempRoom.GetHeight(); ++y)
        {
            if (tempRoom[Vector2u(0, y)] == BT_NONE)
            {
                openSpaces.push_back(Vector2u(0, y));
            }
            if (tempRoom[Vector2u(tempRoom.GetWidth() - 1, y)] == BT_NONE)
            {
                openSpaces.push_back(Vector2u(tempRoom.GetWidth() - 1, y));
            }
        }

        //Get all combinations of endpoints along the edge of the room.
        std::vector<Vector4u> openSpacePairs;
        openSpacePairs.reserve(openSpacePairs.size() * openSpacePairs.size());
        for (unsigned int i = 0; i < openSpaces.size(); ++i)
        {
            for (unsigned int j = i + 1; j < openSpaces.size(); ++j)
            {
                openSpacePairs.push_back(Vector4u(openSpaces[i].x, openSpaces[i].y,
                                                  openSpaces[j].x, openSpaces[j].y));
            }
        }
        std::random_shuffle(openSpacePairs.begin(), openSpacePairs.end());

        //Trim down the number of pairs to a reasonable size.
        const unsigned int MAX_PAIRS = 30;
        if (openSpacePairs.size() > MAX_PAIRS)
        {
            openSpacePairs.resize(MAX_PAIRS);
        }


        //Now go through every pair and get the average path length.

        unsigned int totalSteps = 0;
        unsigned int nPaths = 0;
        LevelGraph graph(tempRoom);
        LevelGraphPather pather(&graph);
        GraphSearchGoal<LevelNode> goal = GraphSearchGoal<LevelNode>(LevelNode(Vector2u()));
        std::vector<LevelNode> dummyPath;

        for (unsigned int i = 0; i < openSpacePairs.size(); ++i)
        {
            Vector2u start(openSpacePairs[i].x, openSpacePairs[i].y),
                     end(openSpacePairs[i].z, openSpacePairs[i].w);

            dummyPath.clear();
            goal.SpecificEnd.SetValue(end);
            if (pather.Search(start, goal, dummyPath))
            {
                totalSteps += dummyPath.size();
                nPaths += 1;
            }
        }
        if (nPaths == 0)
        {
            room.AverageLength = 0.0f;
        }
        else
        {
            room.AverageLength = (float)totalSteps / (float)nPaths;
        }

        #pragma endregion

        roomsToNav -= 1;
    }
    else if (roomsToPath > 0)
    {
        assert(lvl.Rooms.size() >= roomsToPath);

        //Path from the room to the team bases.
        path.clear();
        for (unsigned int i = 0; i < 2; ++i)
        {
            unsigned int teamIndex = (i == 0 ? lvl.Team1Base : lvl.Team2Base);
            
            LevelInfo::RoomData& room = lvl.Rooms[roomsToPath - 1];
            LevelInfo::UIntBox rmBnds = lvl.GetBounds(roomsToPath - 1);
            RoomNode startNode(&room);

            LevelInfo::RoomData& goalRm = lvl.Rooms[teamIndex];
            LevelInfo::UIntBox goalBnds = lvl.GetBounds(teamIndex);
            GraphSearchGoal<RoomNode> goal = GraphSearchGoal<RoomNode>(&goalRm);

            //The pather may fail if a room isn't connected to anything.
            if (pather.Search(startNode, goal, path))
            {
                nStepsFromRoomToTeamBases[roomsToPath - 1][i] = path.size();
            }
            else
            {
                nStepsFromRoomToTeamBases[roomsToPath - 1][i] = std::numeric_limits<unsigned int>::max();
            }
            path.clear();
        }

        roomsToPath -= 1;

        //If we've finally calculated pathing info for all the rooms, finalize pathing data.
        if (roomsToPath == 0)
        {
            //Get the max possible distance from any room to each team base.
            unsigned int maxDists[2] = { std::numeric_limits<unsigned int>::max(),
                                         std::numeric_limits<unsigned int>::max() };
            for (unsigned int i = 0; i < nStepsFromRoomToTeamBases.size(); ++i)
            {
                maxDists[0] = Mathf::Max(maxDists[0], nStepsFromRoomToTeamBases[i][0]);
                maxDists[1] = Mathf::Max(maxDists[1], nStepsFromRoomToTeamBases[i][1]);
            }

            //Get each room's distance to each base, from 0 to 1.
            for (unsigned int i = 0; i < nStepsFromRoomToTeamBases.size(); ++i)
            {
                roomNormalizedDistsToTeamBases[i][0] =
                    (float)nStepsFromRoomToTeamBases[i][0] / (float)maxDists[0];
                roomNormalizedDistsToTeamBases[i][1] =
                    (float)nStepsFromRoomToTeamBases[i][1] / (float)maxDists[1];
            }
        }
    }
}
void GUILevelPathing::Render(float elapsedTime, const RenderInfo& info)
{
    //Overlay a color onto each room based on its proximity to each team.
    for (unsigned int i = 0; i < editor.LevelData.Rooms.size(); ++i)
    {
        const LevelInfo::RoomData& room = editor.LevelData.Rooms[i];

        Vector2f worldCenter = ToV2f(room.MinCornerPos) + (ToV2f(room.Walls.GetDimensions()) * 0.5f);
        SetBounds(Box2D(editor.WorldPosToScreen(worldCenter),
                        editor.WorldSizeToScreen(ToV2f(room.Walls.GetDimensions()))));

        SetColor((COLOR_Team1 * (1.0f - roomNormalizedDistsToTeamBases[i][0])) +
                 (COLOR_Team2 * (1.0f - roomNormalizedDistsToTeamBases[i][1])));

        GUITexture::Render(elapsedTime, info);
    }
}

#undef MC
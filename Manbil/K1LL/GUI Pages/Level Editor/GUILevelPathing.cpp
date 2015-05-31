#include "GUILevelPathing.h"

#include "../../Content/MenuContent.h"

#include "LevelEditor.h"
#include "../../Game/Level/LevelGraph.h"


#define MC MenuContent::Instance


GUILevelPathing::GUILevelPathing(LevelEditor& _editor)
    : editor(_editor), pather(&graph), levelGrid(1, 1, BT_NONE),
      GUITexture(MC.StaticColorGUIParams, 0, MC.StaticColorGUIMat)
{

}

void GUILevelPathing::OnRoomsChanged(void)
{
    LevelInfo::UIntBox bnds = editor.LevelData.GetBounds();
    bnds.Min = Vector2u();
    editor.LevelData.GenerateFullLevel(levelGrid, bnds);

    roomsToPath = editor.LevelData.Rooms.size();
    roomsToNav = roomsToPath;

    nStepsFromRoomToTeamBases.resize(editor.LevelData.Rooms.size(), { 0, 0 });
    roomNormalizedDistsToTeamBases.resize(editor.LevelData.Rooms.size(), { 1.0f, 1.0f });
}

void GUILevelPathing::CustomUpdate(float elapsedTime, Vector2f relativeMouse)
{
    LevelInfo& lvl = editor.LevelData;

    //If rooms need to have their pathing info updated, pick one of them and update it.
    if (roomsToNav > 0)
    {
        assert(roomsToNav <= lvl.Rooms.size());

        //Figure out the average path length to pass through the room, vertically and horizontally.

        LevelInfo::RoomData& room = lvl.Rooms[roomsToNav - 1];
        LevelInfo::UIntBox rmBnds = lvl.GetBounds(roomsToNav - 1);
        LevelGraph graph(levelGrid);
        LevelGraphPather pather(&graph);
        GraphSearchGoal<LevelNode> goal = GraphSearchGoal<LevelNode>(LevelNode(Vector2u()));
        std::vector<LevelNode> dummyPath;

        #pragma region Horizontal

        unsigned int totalSteps = 0;
        unsigned int nPaths = 0;
        for (unsigned int x = rmBnds.Min.x; x <= rmBnds.Max.x; ++x)
        {
            Vector2u startPos(x, rmBnds.Min.y);

            //If this is a used, open doorway, see how long it takes
            //    to reach the doorways on the other side of the room.
            if (levelGrid[startPos] == BT_NONE)
            {
                for (unsigned int x2 = rmBnds.Min.x; x2 <= rmBnds.Max.x; ++x2)
                {
                    Vector2u endPos(x2, rmBnds.Max.y);

                    if (levelGrid[endPos] == BT_NONE)
                    {
                        goal.SpecificEnd.SetValue(LevelNode(endPos));

                        if (!pather.Search(LevelNode(startPos), goal, dummyPath))
                        {
                            totalSteps += dummyPath.size();
                            dummyPath.clear();
                            nPaths += 1;
                        }
                    }
                }
            }
        }
        lvl.Rooms[roomsToNav - 1].NavDifficultyHorz = (float)totalSteps / (float)nPaths;

        #pragma endregion

        #pragma region Vertical

        totalSteps = 0;
        nPaths = 0;
        for (unsigned int y = rmBnds.Min.y; y <= rmBnds.Max.y; ++y)
        {
            Vector2u startPos(rmBnds.Min.x, y);

            //If this is a used, open doorway, see how long it takes
            //    to reach the doorways on the other side of the room.
            if (levelGrid[startPos] == BT_NONE)
            {
                for (unsigned int y2 = rmBnds.Min.y; y2 <= rmBnds.Max.y; ++y2)
                {
                    Vector2u endPos(rmBnds.Max.x, y2);

                    if (levelGrid[endPos] == BT_NONE)
                    {
                        goal.SpecificEnd.SetValue(LevelNode(endPos));
                        if (!pather.Search(LevelNode(startPos), goal, dummyPath))
                        {
                            totalSteps += dummyPath.size();
                            dummyPath.clear();
                            nPaths += 1;
                        }
                    }
                }
            }
        }
        lvl.Rooms[roomsToNav - 1].NavDifficultyVert = (float)totalSteps / (float)nPaths;

        #pragma endregion

        roomsToNav -= 1;
    }
    else if (roomsToPath > 0)
    {
        assert(lvl.Rooms.size() >= roomsToPath);

        //Path from the room to the team bases.
        
        for (unsigned int i = 0; i < 2; ++i)
        {
            unsigned int teamIndex = (i == 0 ? lvl.Team1Base : lvl.Team2Base);
            
            LevelInfo::RoomData& room = lvl.Rooms[roomsToPath - 1];
            LevelInfo::UIntBox rmBnds = lvl.GetBounds(roomsToPath - 1);
            RoomNode startNode(rmBnds.Min, rmBnds.Max, room.SpawnedItem,
                               room.NavDifficultyHorz, room.NavDifficultyVert, roomsToPath - 1);

            path.clear();
            LevelInfo::RoomData& goalRm = lvl.Rooms[teamIndex];
            LevelInfo::UIntBox goalBnds = lvl.GetBounds(teamIndex);
            GraphSearchGoal<RoomNode> goal = GraphSearchGoal<RoomNode>(RoomNode(goalBnds.Min, goalBnds.Max,
                                                                                goalRm.SpawnedItem,
                                                                                goalRm.NavDifficultyHorz,
                                                                                goalRm.NavDifficultyVert,
                                                                                teamIndex));

            //The pather may fail if a room isn't connected to anything.
            if (pather.Search(startNode, goal, path))
            {
                nStepsFromRoomToTeamBases[roomsToPath - 1][i] = std::numeric_limits<unsigned int>::max();
            }
            else
            {
                nStepsFromRoomToTeamBases[roomsToPath - 1][i] = path.size();
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

        SetColor(Vector4f(1.0f, 0.0f, 1.0f, 0.25f)); // Debugging color.
        //TODO: Calculate a real color.

        GUITexture::Render(elapsedTime, info);
    }
}

#undef MC
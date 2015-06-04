#pragma once

#include <array>

#include "../../../Rendering/GUI/GUI Elements/GUITexture.h"

#include "../../Level Info/RoomInfo.h"
#include "../../Game/Level/RoomsGraph.h"


class LevelEditor;

//Calculates/displays pathing information for the level being edited.
class GUILevelPathing : public GUITexture
{
public:

    //If true, this instance will calculate pathing info for one room every frame
    //   until all rooms have had their pathing info calculated.
    bool UpdatePathing = true;


    GUILevelPathing(LevelEditor& editor);


    //Callbacks for changes to the level.
    void OnRoomsChanged(void),
         OnTeamBasesChanged(void);

    virtual void Render(float elapsedTime, const RenderInfo& info) override;


protected:
    
    virtual void CustomUpdate(float elapsedTime, Vector2f relativeMousePos) override;


private:

    LevelEditor& editor;

    Array2D<BlockTypes> levelGrid;
    std::vector<std::array<float, 2>> nStepsFromRoomToTeamBases;
    std::vector<std::array<float, 2>> roomNormalizedDistsToTeamBases;

    RoomsGraph graph;
    RoomsGraphPather pather;
    std::vector<RoomNode> path;

    //The number of rooms left to have their pathing info re-calculated.
    unsigned int roomsToPath = 0,
                 roomsToNav = 0;

    void RecalcPaths(void);
};
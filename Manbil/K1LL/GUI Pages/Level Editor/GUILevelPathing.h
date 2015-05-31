#pragma once

#include "../../../Rendering/GUI/GUI Elements/GUITexture.h"

#include "../../Level Info/RoomInfo.h"
#include "../../Game/Level/RoomsGraph.h"


class LevelEditor;

//Calculates/displays pathing information for the level being edited.
class GUILevelPathing : public GUITexture
{
public:

    GUILevelPathing(LevelEditor& editor);


    //Callbacks for changes to rooms.
    void OnRoomsChanged(void);


protected:
    
    virtual void CustomUpdate(float elapsedTime, Vector2f relativeMousePos) override;
    virtual void Render(float elapsedTime, const RenderInfo& info) override;


private:

    LevelEditor& editor;

    Array2D<BlockTypes> levelGrid;
    std::vector<unsigned int[2]> nStepsFromRoomToTeamBases;
    std::vector<float[2]> roomNormalizedDistsToTeamBases;

    RoomsGraph graph;
    RoomsGraphPather pather;
    std::vector<RoomNode> path;

    //The number of rooms left to have their pathing info re-calculated.
    unsigned int roomsToPath = 0,
                 roomsToNav = 0;

    void RecalcPaths(void);
};
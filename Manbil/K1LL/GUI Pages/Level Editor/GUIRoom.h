#pragma once

#include "../../../Rendering/GUI/GUIElement.h"

#include "../../Level Info/LevelInfo.h"
#include "../../Level Info/ItemTypes.h"
#include "GUIEditorGrid.h"


class LevelEditor;

//A GUI element that renders a room.
class GUIRoom : public GUITexture
{
public:

    const LevelInfo::RoomData* RoomData = 0;

    LevelEditor& Editor;
    const GUIEditorGrid& Grid;

    //If true, this instance is rendering a room that the player is moving.
    bool IsBeingPlaced = false;


    GUIRoom(const GUIEditorGrid& grid, LevelEditor& editor, bool isBeingPlaced,
            const LevelInfo::RoomData* roomData);

    virtual void Render(float elapsedTime, const RenderInfo& info) override;
};
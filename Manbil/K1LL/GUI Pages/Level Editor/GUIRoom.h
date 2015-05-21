#pragma once

#include "../../../Rendering/GUI/GUIElement.h"

#include "../../Level Info/RoomInfo.h"
#include "../../Level Info/ItemTypes.h"
#include "GUIEditorGrid.h"


class LevelEditor;

//A GUI element that renders a room.
class GUIRoom : public GUITexture
{
public:

    const RoomInfo* RoomData = 0;
    ItemTypes RoomSpawn;

    const LevelEditor& Editor;
    const GUIEditorGrid& Grid;

    //If true, this element is rendering a room that the player is moving with the mouse.
    bool IsBeingPlaced = false;


    GUIRoom(const GUIEditorGrid& grid, const LevelEditor& editor, bool isBeingPlaced,
            const RoomInfo* roomData, ItemTypes roomSpawn);

    virtual void Render(float elapsedTime, const RenderInfo& info) override;
};
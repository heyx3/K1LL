#pragma once

#include "../Math/Shapes/Boxes.h"
#include "../IO/DataSerialization.h"

#include "LineSegment.h"
#include "Weapons.h"


struct RoomInfo : public ISerializable
{
    Vector2u GridSize;

    float NavigationDifficulty;

    //Relative to this room's grid.
    std::vector<Vector2u> ConnectingGridPoses;
    std::vector<Vector2f> ItemSpawnPoses;
    std::vector<Box2D> PlayerSpawnRegions;
    std::vector<LineSegment> Walls;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
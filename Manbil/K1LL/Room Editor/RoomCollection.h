#pragma once

#include "../../IO/DataSerialization.h"
#include "../../Editor/IEditable.h"

#include "../Level Info/RoomInfo.h"


struct RoomCollection : public ISerializable
{
public:

    std::vector<RoomInfo> Rooms;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
};
#pragma once

#include "../../IO/DataSerialization.h"


class Constants : public ISerializable
{
public:

    static Constants Instance;


    float CeilingHeight = 10.0f;

    float PlayerCollisionRadius = 0.4f,
          PlayerHeight = 0.9f;
    float PlayerAccel = 3.0f,
          PlayerMaxSpeed = 4.0f;
    float PlayerLookMinDot = 0.1f;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;


    void SaveToFile(std::string& outError) const;
    void ReadFromFile(std::string& outError);


private:

    Constants(void) { }
};
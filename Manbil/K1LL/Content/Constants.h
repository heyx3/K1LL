#pragma once

#include "../../IO/DataSerialization.h"


class Constants : public ISerializable
{
public:

    static Constants Instance;


    float CeilingHeight = 5.0f;

    float PlayerCollisionRadius = 0.4f,
          PlayerHeight = 0.9f;
    float PlayerLookMinDot = 0.01f;
    float PlayerEyeHeight = 1.0f,
          PlayerEyeForward = 0.2f;

    float CameraZNear = 0.001f,
          CameraZFar = 300.0f;

    float PlayerAccel = 50.0f,
          PlayerMaxSpeed = 3.0f;
    float PlayerFriction = 50.0f;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;


    void SaveToFile(std::string& outError) const;
    void ReadFromFile(std::string& outError);


private:

    Constants(void) { }
};
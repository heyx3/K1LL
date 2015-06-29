#pragma once

#include "../../IO/Serialization.h"
#include "../../Math/Lower Math/Vectors.h"


//Game-related constants.
class LevelConstants : public ISerializable
{
public:

    static LevelConstants Instance;


    float CeilingHeight = 5.0f;

    float PlayerCollisionRadius = 0.4f,
          PlayerHeight = 0.9f;
    
    Vector3f PlayerStartLookDir = Vector3f(1.0f, 0.0f, 0.0f);

    float PlayerLookMinDot = 0.01f;
    float PlayerEyeHeight = 1.05f,
          PlayerEyeForward = 0.2f;

    float CameraZNear = 0.001f,
          CameraZFar = 300.0f;

    float PlayerAccel = 50.0f,
          PlayerMaxSpeed = 3.0f;
    float PlayerFriction = 50.0f;


    Vector3f GetPlayerEyePos(Vector2f pos, Vector3f lookDir) const;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;


    void SaveToFile(std::string& outError) const;
    void ReadFromFile(std::string& outError);


private:

    LevelConstants(void) { }
};
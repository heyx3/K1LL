#pragma once

#include "../../IO/DataSerialization.h"


//TODO: Also inherit from IEditable for the options menu.
//Contains all the constants that are customizable in the Options menu.
class Settings : public ISerializable
{
public:

    static Settings Instance;


    const float MinMouseSpeed = 0.001f,
                MaxMouseSpeed = 0.2f;
    const float MinThumbstickSpeed = 0.1f,
                MaxThumbstickSpeed = 1.0f;
    const float MinFOVDegrees = 55.0f,
                MaxFOVDegrees = 110.0f;


    float MouseSpeedX = 0.01f,
          MouseSpeedY = 0.01f;
    float ThumbstickSpeedX = 0.25f,
          ThumbstickSpeedY = 0.25f;

    float MasterVolume = 1.0f;

    float FOVDegrees = 80.0f;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
    
    //Reads from the "Settings" file if it's there.
    //If it's not there, creates a settings file with the default values.
    void Initialize();
    void SaveToFile(std::string& outError) const;


private:

    Settings(void) { }
};
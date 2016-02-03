#pragma once

#include "../../IO/DataSerialization.h"


//TODO: Also inherit from IEditable for the options menu.
//Contains all the constants that are customizable in the Quality Options menu.
class QualitySettings : public ISerializable
{
public:

    static QualitySettings Instance;


    //TODO: Check this value against the max texture size according to the OpenGL context.
    unsigned int MaxParticles = 4096;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;
    
    //Reads from the "GraphicsSettings" file if it's there.
    //If it's not there, creates such a file with default values.
    void Initialize();
    void SaveToFile(std::string& outError) const;


private:

    QualitySettings(void) { }
};
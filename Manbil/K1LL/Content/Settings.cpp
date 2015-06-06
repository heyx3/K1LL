#include "Settings.h"

#include <iostream>

#include "../../IO/XmlSerialization.h"


namespace
{
    std::string FILE_PATH = "Content/Settings.xml";


    bool Assert(bool val, const std::string& errIntro, const std::string& errBody,
                std::string& outFullError, bool print)
    {
        if (!val)
        {
            outFullError = errIntro + ": " + errBody;
            if (print)
            {
                std::cout << outFullError << "\n\n";
                char pauser;
                std::cin >> pauser;
            }
        }

        return val;
    }
}


Settings Settings::Instance = Settings();


void Settings::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(MouseSpeedX, "Mouse Speed X");
    writer->WriteFloat(MouseSpeedY, "Mouse Speed Y");
    writer->WriteFloat(ThumbstickSpeedX, "Thumbstick Speed X");
    writer->WriteFloat(ThumbstickSpeedY, "Thumbstick Speed Y");

    writer->WriteFloat(MasterVolume, "Master Volume");
}
void Settings::ReadData(DataReader* reader)
{
    reader->ReadFloat(MouseSpeedX);
    reader->ReadFloat(MouseSpeedY);
    reader->ReadFloat(ThumbstickSpeedX);
    reader->ReadFloat(ThumbstickSpeedY);

    reader->ReadFloat(MasterVolume);
}

void Settings::SaveToFile(std::string& outErr) const
{
    //Try writing out the settings.

    XmlWriter writer;
    writer.WriteDataStructure(*this, "Values");

    std::string err = writer.SaveData(FILE_PATH);
    Assert(err.empty(), "Error saving settings to '" + FILE_PATH + "'", err, outErr, false);
}
void Settings::ReadFromFile()
{
    //Try loading in the settings.

    XmlReader reader(FILE_PATH);

    //It's not a fatal error if the file isn't there.
    std::string err;
    if (!Assert(reader.ErrorMessage.empty(), "Error reading in '" + FILE_PATH + "'",
                reader.ErrorMessage, err, true))
    {
        std::cout << "\nUsing default settings.\n";
        return;
    }
    
    try
    {
        reader.ReadDataStructure(*this);
    }
    catch (int e)
    {
        assert(e == DataReader::EXCEPTION_FAILURE);
        Assert(false, "Error parsing '" + FILE_PATH + "'", reader.ErrorMessage, err, true);
    }
}
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

    writer->WriteFloat(FOVDegrees, "FOV in degrees");
}
void Settings::ReadData(DataReader* reader)
{
    reader->ReadFloat(MouseSpeedX);
    reader->ReadFloat(MouseSpeedY);
    reader->ReadFloat(ThumbstickSpeedX);
    reader->ReadFloat(ThumbstickSpeedY);

    reader->ReadFloat(MasterVolume);

    reader->ReadFloat(FOVDegrees);
}

void Settings::Initialize()
{
    //Try loading in the settings.

    XmlReader reader(FILE_PATH);

    //If the file isn't there, write it out and print a little notice.
    if (!reader.ErrorMessage.empty())
    {
        std::cout << "\nCouldn't read in settings; overwriting with default values.\n\n";
        
        std::string err, errFull;
        SaveToFile(err);
        if (!Assert(err.empty(), "Error saving file to '" + FILE_PATH + "'", err, errFull, true))
        {
            return;
        }

        reader.ErrorMessage.clear();
        reader.Reload(FILE_PATH, reader.ErrorMessage);
        if (!Assert(reader.ErrorMessage.empty(), "Error reading file '" + FILE_PATH + "'",
                    reader.ErrorMessage, errFull, true))
        {
            return;
        }
    }
    
    try
    {
        reader.ReadDataStructure(*this);
    }
    catch (int e)
    {
        assert(e == DataReader::EXCEPTION_FAILURE);
        std::string err;
        Assert(false, "Error parsing '" + FILE_PATH + "'", reader.ErrorMessage, err, true);
    }
}
void Settings::SaveToFile(std::string& outErr) const
{
    //Try writing out the settings.

    XmlWriter writer;
    writer.WriteDataStructure(*this, "Values");

    std::string err = writer.SaveData(FILE_PATH);
    Assert(err.empty(), "Error saving settings to '" + FILE_PATH + "'", err, outErr, false);
}
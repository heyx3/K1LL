#include "QualitySettings.h"

#include <iostream>

#include "../../IO/XmlSerialization.h"


namespace
{
    std::string FILE_PATH = "Content/QualitySettings.xml";


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


QualitySettings QualitySettings::Instance = QualitySettings();


void QualitySettings::WriteData(DataWriter* writer) const
{
    writer->WriteUInt(MaxParticles, "Max Particles");
}
void QualitySettings::ReadData(DataReader* reader)
{
    reader->ReadUInt(MaxParticles);
}

void QualitySettings::Initialize()
{
    //Try loading in the quality settings.

    XmlReader reader(FILE_PATH);

    //If the file isn't there, write it out and print a little notice.
    if (!reader.ErrorMessage.empty())
    {
        std::cout << "\nCouldn't read in quality settings; overwriting with default values.\n\n";
        
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
void QualitySettings::SaveToFile(std::string& outErr) const
{
    //Try writing out the settings.

    XmlWriter writer;
    writer.WriteDataStructure(*this, "Values");

    std::string err = writer.SaveData(FILE_PATH);
    Assert(err.empty(), "Error saving quality settings to '" + FILE_PATH + "'", err, outErr, false);
}
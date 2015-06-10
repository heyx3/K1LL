#include "Constants.h"


#include "../../IO/XmlSerialization.h"


Constants Constants::Instance = Constants();


void Constants::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(CeilingHeight, "Ceiling height");
    writer->WriteFloat(PlayerCollisionRadius, "Player collision radius");
    writer->WriteFloat(PlayerHeight, "Player height");
    writer->WriteFloat(PlayerEyeHeight, "Player eye height");
    writer->WriteFloat(PlayerEyeForward, "Player eye forward");
    writer->WriteFloat(PlayerAccel, "Player acceleration");
    writer->WriteFloat(PlayerMaxSpeed, "Player max speed");
    writer->WriteFloat(PlayerLookMinDot, "Player look min dot product");
    writer->WriteFloat(CameraZNear, "Camera near Z plane");
    writer->WriteFloat(CameraZFar, "Camera far Z plane");
}

void Constants::ReadData(DataReader* reader)
{
    reader->ReadFloat(CeilingHeight);
    reader->ReadFloat(PlayerCollisionRadius);
    reader->ReadFloat(PlayerHeight);
    reader->ReadFloat(PlayerEyeHeight);
    reader->ReadFloat(PlayerEyeForward);
    reader->ReadFloat(PlayerAccel);
    reader->ReadFloat(PlayerMaxSpeed);
    reader->ReadFloat(PlayerLookMinDot);
    reader->ReadFloat(CameraZNear);
    reader->ReadFloat(CameraZFar);
}


void Constants::SaveToFile(std::string& err) const
{
    XmlWriter writer;

    try
    {
        writer.WriteDataStructure(*this, "Data");
        err = writer.SaveData("Content/Constants.xml");
    }
    catch (int ex)
    {
        assert(ex == DataWriter::EXCEPTION_FAILURE);
        err = "Error saving data: " + writer.ErrorMessage;
    }
}
void Constants::ReadFromFile(std::string& err)
{
    XmlReader reader("Content/Constants.xml");

    err = reader.ErrorMessage;
    if (!err.empty())
    {
        err = "Error reading file: " + err;
        return;
    }

    try
    {
        reader.ReadDataStructure(*this);
    }
    catch (int ex)
    {
        assert(ex == DataReader::EXCEPTION_FAILURE);
        err = "Error reading data: " + reader.ErrorMessage;
    }
}
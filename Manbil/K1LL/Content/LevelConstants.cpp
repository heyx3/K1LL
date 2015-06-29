#include "LevelConstants.h"


#include "../../IO/XmlSerialization.h"


LevelConstants LevelConstants::Instance = LevelConstants();


Vector3f LevelConstants::GetPlayerEyePos(Vector2f pos, Vector3f lookDir) const
{
    return Vector3f(pos, PlayerEyeHeight) +
           Vector3f(lookDir.XY().Normalized() * PlayerEyeForward, 0.0f);
}

void LevelConstants::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(CeilingHeight, "Ceiling height");

    writer->WriteFloat(PlayerCollisionRadius, "Player collision radius");
    writer->WriteFloat(PlayerHeight, "Player height");

    writer->WriteDataStructure(Vector3f_Writable(PlayerStartLookDir),
                               "Starting look direction for players");
    writer->WriteFloat(PlayerEyeHeight, "Player eye height");
    writer->WriteFloat(PlayerEyeForward, "Player eye forward");
    writer->WriteFloat(PlayerLookMinDot, "Player look min dot product");

    writer->WriteFloat(CameraZNear, "Camera near Z plane");
    writer->WriteFloat(CameraZFar, "Camera far Z plane");

    writer->WriteFloat(PlayerAccel, "Player acceleration");
    writer->WriteFloat(PlayerMaxSpeed, "Player max speed");
    writer->WriteFloat(PlayerFriction, "Player friction");
}

void LevelConstants::ReadData(DataReader* reader)
{
    reader->ReadFloat(CeilingHeight);

    reader->ReadFloat(PlayerCollisionRadius);
    reader->ReadFloat(PlayerHeight);

    reader->ReadDataStructure(Vector3f_Readable(PlayerStartLookDir));
    reader->ReadFloat(PlayerEyeHeight);
    reader->ReadFloat(PlayerEyeForward);
    reader->ReadFloat(PlayerLookMinDot);

    reader->ReadFloat(CameraZNear);
    reader->ReadFloat(CameraZFar);

    reader->ReadFloat(PlayerAccel);
    reader->ReadFloat(PlayerMaxSpeed);
    reader->ReadFloat(PlayerFriction);
}


void LevelConstants::SaveToFile(std::string& err) const
{
    XmlWriter writer;

    try
    {
        writer.WriteDataStructure(*this, "Data");
        err = writer.SaveData("Content/LevelConstants.xml");
    }
    catch (int ex)
    {
        assert(ex == DataWriter::EXCEPTION_FAILURE);
        err = "Error saving data: " + writer.ErrorMessage;
    }
}
void LevelConstants::ReadFromFile(std::string& err)
{
    XmlReader reader("Content/LevelConstants.xml");

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
#include "WeaponConstants.h"


#include "../../IO/XmlSerialization.h"


WeaponConstants WeaponConstants::Instance = WeaponConstants();


void WeaponConstants::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(WeaponLength, "Weapon length");
    writer->WriteDataStructure(Vector3f_Writable(WeaponForward), "Starting forward");
    writer->WriteDataStructure(Vector3f_Writable(WeaponOffset), "Starting offset");

    writer->WriteFloat(PuncherReloadTime, "Puncher: reload time");
    writer->WriteFloat(PuncherAnimSpeed, "Puncher: animation speed");
    writer->WriteUInt(PuncherBufferSize, "Puncher: Number of pre-allocated bullets");

    writer->WriteFloat(LightGunAnimSpeed, "LightGun: animation speed");

    writer->WriteFloat(PRGAnimSpeed, "Perpetual Rail Gun: animation speed");

    writer->WriteFloat(TerribleShotgunAnimSpeed, "Terrible Shotgun: animation speed");

    writer->WriteFloat(SprayNPrayAnimSpeed, "Spray N Pray: animation speed");

    writer->WriteFloat(ClusterAnimSpeed, "Cluster: animation speed");

    writer->WriteFloat(POSAnimSpeed, "Personal Orbital Strike: animation speed");
}

void WeaponConstants::ReadData(DataReader* reader)
{
    reader->ReadFloat(WeaponLength);
    reader->ReadDataStructure(Vector3f_Readable(WeaponForward));
    reader->ReadDataStructure(Vector3f_Readable(WeaponOffset));

    reader->ReadFloat(PuncherReloadTime);
    reader->ReadFloat(PuncherAnimSpeed);
    reader->ReadUInt(PuncherBufferSize);

    reader->ReadFloat(LightGunAnimSpeed);

    reader->ReadFloat(PRGAnimSpeed);

    reader->ReadFloat(TerribleShotgunAnimSpeed);

    reader->ReadFloat(SprayNPrayAnimSpeed);

    reader->ReadFloat(ClusterAnimSpeed);

    reader->ReadFloat(POSAnimSpeed);
}


void WeaponConstants::SaveToFile(std::string& err) const
{
    XmlWriter writer;

    try
    {
        writer.WriteDataStructure(*this, "Data");
        err = writer.SaveData("Content/WeaponConstants.xml");
    }
    catch (int ex)
    {
        assert(ex == DataWriter::EXCEPTION_FAILURE);
        err = "Error saving data: " + writer.ErrorMessage;
    }
}
void WeaponConstants::ReadFromFile(std::string& err)
{
    XmlReader reader("Content/WeaponConstants.xml");

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
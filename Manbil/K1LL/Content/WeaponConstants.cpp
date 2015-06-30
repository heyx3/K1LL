#include "WeaponConstants.h"


#include "../../IO/XmlSerialization.h"



void WeaponConstants::MaterialParams::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(AnimSpeed, "Animation speed");
    writer->WriteFloat(GridThinness, "Grid thinness");
    writer->WriteFloat(GridScale, "Grid scale");
    writer->WriteDataStructure(Vector3f_Writable(BulletColor), "Bullet color");
}
void WeaponConstants::MaterialParams::ReadData(DataReader* reader)
{
    reader->ReadFloat(AnimSpeed);
    reader->ReadFloat(GridThinness);
    reader->ReadFloat(GridScale);
    reader->ReadDataStructure(Vector3f_Readable(BulletColor));
}


WeaponConstants WeaponConstants::Instance = WeaponConstants();


void WeaponConstants::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(WeaponLength, "Weapon length");
    writer->WriteDataStructure(Vector3f_Writable(WeaponForward), "Starting forward");
    writer->WriteDataStructure(Vector3f_Writable(WeaponOffset), "Starting offset");

    writer->WriteFloat(PuncherReloadTime, "Puncher: reload time");
    writer->WriteFloat(PuncherBulletSpeed, "Puncher: bullet speed");
    writer->WriteUInt(PuncherBufferSize, "Puncher: Number of pre-allocated bullets");
    
    writer->WriteDataStructure(PuncherMaterial, "Puncher material");
    writer->WriteDataStructure(LightGunMaterial, "Light Gun material");
    writer->WriteDataStructure(PRGMaterial, "Perpetual Rail Gun material");
    writer->WriteDataStructure(TerribleShotgunMaterial, "Terrible Shotgun material");
    writer->WriteDataStructure(SprayNPrayMaterial, "Spray N Pray material");
    writer->WriteDataStructure(ClusterMaterial, "Cluster material");
    writer->WriteDataStructure(POSMaterial, "Personal Orbital Strike material");
}

void WeaponConstants::ReadData(DataReader* reader)
{
    reader->ReadFloat(WeaponLength);
    reader->ReadDataStructure(Vector3f_Readable(WeaponForward));
    reader->ReadDataStructure(Vector3f_Readable(WeaponOffset));

    reader->ReadFloat(PuncherReloadTime);
    reader->ReadFloat(PuncherBulletSpeed);
    reader->ReadUInt(PuncherBufferSize);
    
    reader->ReadDataStructure(PuncherMaterial);
    reader->ReadDataStructure(LightGunMaterial);
    reader->ReadDataStructure(PRGMaterial);
    reader->ReadDataStructure(TerribleShotgunMaterial);
    reader->ReadDataStructure(SprayNPrayMaterial);
    reader->ReadDataStructure(ClusterMaterial);
    reader->ReadDataStructure(POSMaterial);
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
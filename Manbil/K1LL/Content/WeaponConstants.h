#pragma once

#include "../../IO/Serialization.h"
#include "../../Math/Lower Math/Vectors.h"


class WeaponConstants : public ISerializable
{
public:

    static WeaponConstants Instance;


    float WeaponLength = 0.19f;
    Vector3f WeaponForward = Vector3f(1.0f, 0.0f, 0.0f);
    Vector3f WeaponOffset = Vector3f(0.1f, 0.0f, 0.5f);

    float PuncherReloadTime = 0.185f;
    float PuncherAnimSpeed = 1.0f;
    unsigned int PuncherBufferSize = 100;

    float LightGunAnimSpeed = 1.0f;

    float PRGAnimSpeed = 1.0f;

    float TerribleShotgunAnimSpeed = 1.0f;

    float SprayNPrayAnimSpeed = 1.0f;

    float ClusterAnimSpeed = 1.0f;

    float POSAnimSpeed = 1.0f;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;


    void SaveToFile(std::string& outError) const;
    void ReadFromFile(std::string& outError);


private:

    WeaponConstants(void) { }
};
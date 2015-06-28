#pragma once

#include "../../IO/Serialization.h"
#include "../../Math/Lower Math/Vectors.h"


class WeaponConstants : public ISerializable
{
public:

    struct MaterialParams : public ISerializable
    {
        float AnimSpeed = 0.15f;
        float GridThinness = 45.0f;
        float GridScale = 6.5f;


        MaterialParams(void) { }
        MaterialParams(float animSpeed, float gridThinness, float gridScale)
            : AnimSpeed(animSpeed), GridThinness(gridThinness), GridScale(gridScale) { }


        virtual void WriteData(DataWriter* writer) const override;
        virtual void ReadData(DataReader* reader) override;
    };


    static WeaponConstants Instance;


    float WeaponLength = 0.19f;
    Vector3f WeaponForward = Vector3f(1.0f, 0.0f, 0.0f);
    Vector3f WeaponOffset = Vector3f(0.3f, 0.115f, 0.9f);

    float PuncherReloadTime = 0.185f;
    float PuncherBulletSpeed = 5.0f;
    unsigned int PuncherBufferSize = 100;

    MaterialParams PuncherMaterial, LightGunMaterial, PRGMaterial, TerribleShotgunMaterial,
                   SprayNPrayMaterial, ClusterMaterial, POSMaterial;


    virtual void WriteData(DataWriter* writer) const override;
    virtual void ReadData(DataReader* reader) override;


    void SaveToFile(std::string& outError) const;
    void ReadFromFile(std::string& outError);


private:

    WeaponConstants(void) { }
};
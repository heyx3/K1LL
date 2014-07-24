#pragma once

#include "../Math/Higher Math/Camera.h"
#include "../MovingCamera.h"


//The camera for the PlanetSim project.
class PlanetSimCamera : public MovingCamera
{
public:

    PlanetSimCamera(Vector3f pos, float moveSpeed, float rotSpeed,
                    Vector3f forward = Vector3f(1.0f, 0.0f, 0.0f), Vector3f up = Vector3f(0.0f, 0.0f, 1.0f))
        : MovingCamera(pos, moveSpeed, rotSpeed, forward, up)
    {

    }

    virtual bool Update(float elapsedSeconds) override;
};
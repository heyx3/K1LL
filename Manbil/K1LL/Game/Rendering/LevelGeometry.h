#pragma once

#include "../../../Rendering/Rendering.hpp"

#include "../Actor.h"
#include "../../Level Info/RoomInfo.h"


//Generates and renders the walls, ceiling, and floor.
class LevelGeometry : public Actor
{
public:

    LevelGeometry(Level* theLevel, std::string& outErrorMsg);
    ~LevelGeometry(void);


    virtual bool Update(float elapsedTime) override;
    virtual void Render(float elapsedTime, const RenderInfo& info) override;


private:

    Material* mat;
    Mesh mesh;
    UniformDictionary params;
};
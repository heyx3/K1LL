#pragma once

#include "../../../Rendering/Rendering.hpp"

#include "../Actor.h"
#include "../../Level Info/RoomInfo.h"


//Generates and renders the walls, ceiling, and floor.
class LevelGeometry : public Actor
{
public:

    LevelGeometry(const Array2D<BlockTypes>& levelGrid, std::string& outErrorMsg);
    ~LevelGeometry(void);


    virtual bool Update(Level* theLevel, float elapsedTime) override;
    virtual void Render(Level* theLevel, float elapsedTime, const RenderInfo& info) override;


private:

    Material* mat;
    Mesh mesh;
    UniformDictionary params;
};
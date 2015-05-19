#pragma once

#include "../../../Rendering/GUI/GUI Elements/GUITexture.h"


//A GUIElement that draws an editor's world grid.
class GUIEditorGrid : public GUITexture
{
public:

    Box2D& WorldViewBounds;


    //If something went wrong, an error message is output to the given string.
    GUIEditorGrid(Box2D& worldViewBounds, std::string& outErrorMsg);

    ~GUIEditorGrid(void);


    virtual void Render(float elapsedTime, const RenderInfo& info) override;


private:
    
    static Material* gridMat;
    static UniformDictionary gridMatParams;
    static unsigned int nInstances;
};
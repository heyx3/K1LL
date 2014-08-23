#pragma once

#include "../GUIElement.h"
#include "../TextRenderer.h"
#include "../../Materials/Data Nodes/ShaderGenerator.h"


//Represents a simple piece of text.
class GUILabel : public GUIElement
{
public:

    
    enum HorizontalOffsets
    {
        HO_LEFT,
        HO_CENTER,
        HO_RIGHT,
    };
    enum VerticalOffsets
    {
        VO_TOP,
        VO_CENTER,
        VO_BOTTOM,
    };


    HorizontalOffsets OffsetHorz;
    VerticalOffsets OffsetVert;

    TextRenderer * TextRender;
    TextRenderer::FontSlot TextRenderSlot;

    Material * RenderMat;

    Vector2f Scale;


    virtual Vector2f GetCollisionCenter(void) const override;
    virtual Vector2f GetCollisionDimensions(void) const override { return dimensions.ComponentProduct(Scale); }

    virtual void MoveElement(Vector2f moveAmount) override { center += moveAmount; }
    virtual void SetPosition(Vector2f newPos) override { center = newPos; }

    virtual Vector2f GetScale(void) const override { return Scale; }

    virtual void ScaleBy(Vector2f scaleAmount) override { Scale.MultiplyComponents(scaleAmount); }
    virtual void SetScale(Vector2f newScale) override { Scale = newScale; }

    
    //Starts out with no text (an empty string).
    GUILabel(TextRenderer * textRenderer = 0, TextRenderer::FontSlot textSlot = TextRenderer::FontSlot(),
             Material * material = 0, float timeSpeed = 1.0f,
             HorizontalOffsets offsetH = HO_LEFT, VerticalOffsets offsetV = VO_TOP)
        : OffsetHorz(offsetH), OffsetVert(offsetV), TextRender(textRenderer),
          RenderMat(material), TextRenderSlot(textSlot), text(""), GUIElement(timeSpeed),
          Scale(1.0f, 1.0f)
    {

    }


    const std::string & GetText(void) const { return text; }
    bool SetText(std::string newText);

    virtual std::string Render(float elapsedTime, const RenderInfo & info) override;


private:

    Vector2f GetTextOffset(void) const;

    Vector2f center, dimensions;
    std::string text;
};
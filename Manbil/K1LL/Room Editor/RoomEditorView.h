#pragma once

#include "../../Rendering/GUI/GUIElement.h"
#include "../../Rendering/GUI/GUI Elements/GUITexture.h"
#include "../../Editor/IEditable.h"

#include "../Level Info/RoomInfo.h"
#include "RoomEditorPane.h"


//The GUI element that updates/renders 
class RoomEditorView : public GUIElement, public RoomEditorPane
{
public:

    //Add/subtract this value whenever the mouse wheel changes.
    int MouseWheelDelta;


    RoomEditorView(std::string& outError);
    virtual ~RoomEditorView(void);


    virtual Box2D GetBounds(void) const override;

    virtual void ScaleBy(Vector2f scaleAmount) override;
    virtual void SetScale(Vector2f newScale) override;


    virtual void Render(float elapsedTime, const RenderInfo& info) override;

    virtual void OnMouseClick(Vector2f relativeMousePos) override;
    virtual void OnMouseDrag(Vector2f originalPos, Vector2f newPos) override;
    virtual void OnMouseRelease(Vector2f relativeMousePos) override;


protected:

    virtual void CustomUpdate(float elapsed, Vector2f relativeMousePos) override;

private:

    enum MouseState
    {
        //Mouse is released.
        MS_NONE,
        //Mouse is making blocks everywhere it touches.
        MS_MAKING_BLOCKS,
        //Mouse is dragging the camera.
        MS_DRAGGING_CAMERA,
    };


    MouseState mouseState = MS_NONE;

    Vector2f mouseDrawWorldPos, lastDragWorldPos;

    Vector2f worldPosCenter;
    float viewScale;

    GUITexture GUIBackground, GUIBox;
    MTexture2D backgroundTex;


    Vector2f GetWorldPos(Vector2f relativeGUIPos) const;
    Vector2f GetRelativePos(Vector2f worldPos) const;

    Vector2f GetClosestGridPos(Vector2f worldPos) const;

    Box2D GetWorldBounds(Box2D screenBounds) const;

    Box2D GetGridBounds(void) const;

    bool IsValidGridPos(Vector2f gridPos) const;


    void RenderBox(Vector2f worldCenter, Vector2f worldSize, Vector4f color,
                   float elapsedTime, const RenderInfo& info);
    void RenderLine(Vector2f worldStart, Vector2f worldEnd, float worldThickness,
                    Vector4f color, float elapsedTime, const RenderInfo& info);
};
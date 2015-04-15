#pragma once

#include "../../Rendering/GUI/GUIElement.h"
#include "../../Rendering/GUI/GUI Elements/GUITexture.h"

#include "../RoomInfo.h"


class EditorView : public GUIElement
{
public:

    RoomInfo RoomData;

    //Add/subtract this value whenever the mouse wheel changes.
    int MouseWheelDelta;


    EditorView(std::string& outError);
    virtual ~EditorView(void);


    virtual Box2D GetBounds(void) const override;

    virtual void Render(float elapsedTime, const RenderInfo& info) override;

    virtual void OnMouseClick(Vector2f relativeMousePos) override;
    virtual void OnMouseDrag(Vector2f originalPos, Vector2f newPos) override;
    virtual void OnMouseRelease(Vector2f relativeMousePos) override;


protected:

    virtual void CustomUpdate(float elapsed, Vector2f relativeMousePos) override;


private:

    enum MouseState
    {
        MS_NONE,
        MS_CLICKING,
        MS_DRAGGING_CAMERA,
        MS_DRAGGING_VERTEX,
    };

    struct Vertex
    {
        int SegmentIndex;
        bool IsEnd;
        Vertex(unsigned int segIndex, bool isEnd) : SegmentIndex(segIndex), IsEnd(isEnd) { }
    };


    MouseState mouseState = MS_NONE;
    Vertex vertexBeingDragged = Vertex(-1, false);


    Vector2f worldPosCenter;
    float viewScale;

    Material *lineMat;
    UniformDictionary lineMatParams;

    Material *backgroundMat;
    GUITexture GUIBackground;


    Vector2f GetWorldPos(Vector2f relativeMousePos) { return worldPosCenter + (relativeMousePos / viewScale); }
    Vector2f GetRelativePos(Vector2f worldPos) { return ((worldPos - worldPosCenter) * viewScale); }

    Box2D GetWorldBounds(Box2D screenBounds) { return Box2D(GetWorldPos(screenBounds.GetPosition()), screenBounds.GetDimensions() / viewScale); }

    void RenderLine(Vector2f worldStart, Vector2f worldEnd, float elapsedSeconds, const RenderInfo& info);
};
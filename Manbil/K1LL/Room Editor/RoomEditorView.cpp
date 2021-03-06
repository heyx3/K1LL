#include "RoomEditorView.h"

#include <SFML/Window.hpp>

#include "../../Rendering/Data Nodes/DataNodes.hpp"
#include "../../Rendering/Data Nodes/ShaderGenerator.h"

#include "../../Rendering/GUI/GUIMaterials.h"


const float ZoomSpeed = 1.1f;
const std::string uniform_minWorldPos = "u_minWorldPos",
                  uniform_maxWorldPos = "u_maxWorldPos";


Vector2f RoundToInt(Vector2f inV)
{
    return Vector2f((float)Mathf::RoundToInt(inV.x),
                    (float)Mathf::RoundToInt(inV.y));
}


RoomEditorView::RoomEditorView(std::string& err)
    : viewScale(50.0f), worldPosCenter(5.0f, 5.0f), MouseWheelDelta(0),
      backgroundTex(TextureSampleSettings2D(FT_NEAREST, WT_WRAP), PS_8U, false),
      GUIElement(UniformDictionary())
{
    //Certain things are common to all materials used in this editor.

    RenderIOAttributes quadVertIns = DrawingQuad::GetVertexInputData();
    SerializedMaterial matData(quadVertIns);

    DataLine vIn_Pos(VertexInputNode::GetInstance(), 0),
             vIn_UV(VertexInputNode::GetInstance(), 1);

    DataNode::Ptr objToScreen = SpaceConverterNode::ObjPosToScreenPos(vIn_Pos, "objPosToScreen");
    DataLine screenPos4(objToScreen, 1);
    matData.MaterialOuts.VertexPosOutput = screenPos4;

    DataNode::Ptr colorUniform(new ParamNode(4, GUIMaterials::QuadDraw_Color));
    

    //Background material.

    DataNode::Ptr minWorldPos(new ParamNode(2, uniform_minWorldPos)),
                  maxWorldPos(new ParamNode(2, uniform_maxWorldPos));

    DataNode::Ptr objToWorld(new CustomExpressionNode("mix('0', '1', 0.5 + (0.5 * '2'.xy))",
                                                      2, minWorldPos, maxWorldPos, vIn_Pos,
                                                      "objPosToWorld"));

    DataNode::Ptr vOut_UV(new CustomExpressionNode("0.5 + (0.5 * '0'.xy)", 2, vIn_Pos, "vOut_UV"));

    matData.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_WorldPos", objToWorld));
    matData.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_UV", vOut_UV));

    DataLine fIn_WorldPos(FragmentInputNode::GetInstance(), 0),
             fIn_UV(FragmentInputNode::GetInstance(), 1);

    DataNode::Ptr distToOrigin(new DistanceNode(fIn_WorldPos, Vector2f(0.0f, 0.0f), "distToOrigin")),
                  originStrength(new CustomExpressionNode("max(0.0, pow(1.0 - (min('0', 3.0) / 3.0), 4.5))",
                                                          1, distToOrigin, "originStrength"));
    DataNode::Ptr gridStrength(new CustomExpressionNode("pow(2.0 * abs(fract(abs('0')) - 0.5), vec2(6.5))",
                                                        2, fIn_WorldPos, "gridStrength"));

    DataNode::Ptr texPtr(new TextureSample2DNode(fIn_UV, GUIMaterials::QuadDraw_Texture2D, "texNode"));
    DataLine texRGBA(texPtr, TextureSample2DNode::GetOutputIndex(CO_AllChannels));

    std::string colExpr = "'2' * '3' * vec4(vec3(min('0' + max('1'.x, '1'.y), 1.0)), 1.0)";
    DataNode::Ptr finalCol(new CustomExpressionNode(colExpr, 4,
                                                    originStrength, gridStrength, colorUniform, texRGBA,
                                                    "finalCol"));

    matData.MaterialOuts.FragmentOutputs.push_back(ShaderOutput("fOut_Color", finalCol));

    ShaderGenerator::GeneratedMaterial genM = ShaderGenerator::GenerateMaterial(matData,
                                                                                GUIBackground.Params,
                                                                                BlendMode::GetOpaque());
    if (!genM.ErrorMessage.empty())
    {
        err = "Error creating background GUI material: " + genM.ErrorMessage;
        return;
    }
    GUIBackground.Mat = genM.Mat;


    //Box material.

    matData.MaterialOuts.VertexOutputs.clear();
    matData.MaterialOuts.FragmentOutputs.clear();
    matData.MaterialOuts.FragmentOutputs.push_back(ShaderOutput("fOut_Color", colorUniform));

    genM = ShaderGenerator::GenerateMaterial(matData, GUIBox.Params, BlendMode::GetTransparent());
    if (!genM.ErrorMessage.empty())
    {
        err = "Error creating box GUI material: " + genM.ErrorMessage;
        return;
    }
    GUIBox.Mat = genM.Mat;

    
    //Now create the background texture.

    backgroundTex.Create();
    if (!backgroundTex.SetDataFromFile("Content/Room Editor/Background.png", err))
    {
        err = "Error loading background tex: " + err;
        return;
    }
    GUIBackground.SetTex(&backgroundTex);
    GUIBackground.SetColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));


    err = LoadData();
    if (!err.empty())
    {
        err.clear();
        Rooms.Rooms.clear();
        Rooms.Rooms.push_back(RoomInfo());
        Rooms.Rooms[0].RoomGrid.Reset(5, 5, BT_WALL);
    }
    assert(Rooms.Rooms.size() > 0);
}

RoomEditorView::~RoomEditorView(void)
{
    if (GUIBackground.Mat != 0)
    {
        delete GUIBackground.Mat;
    }
    if (GUIBox.Mat != 0)
    {
        delete GUIBox.Mat;
    }
}

Box2D RoomEditorView::GetBounds(void) const
{
    Box2D bnds = GUIBackground.GetBounds();
    bnds.Move(GUIBackground.GetPos());
    return bnds;
}


void RoomEditorView::ScaleBy(Vector2f scaleAmount)
{
    GUIBackground.ScaleBy(scaleAmount);
    GUIBox.ScaleBy(scaleAmount);
    GUIElement::ScaleBy(scaleAmount);
}
void RoomEditorView::SetScale(Vector2f newScale)
{
    GUIBackground.SetScale(newScale);
    GUIBox.SetScale(newScale);
    GUIElement::SetScale(newScale);
}

void RoomEditorView::CustomUpdate(float elapsed, Vector2f mousePos)
{
    if (MouseWheelDelta != 0)
    {
        viewScale *= powf(ZoomSpeed, (float)MouseWheelDelta);
        MouseWheelDelta = 0;
    }

    mouseDrawWorldPos = GetClosestGridPos(GetWorldPos(mousePos) - Vector2f(0.5f, 0.5f));
}

void RoomEditorView::Render(float elapsedTime, const RenderInfo& info)
{
    //Render the background.
    Box2D worldBounds = GetWorldBounds(GetBounds());
    Vector2f min = worldBounds.GetMinCorner(),
             max = worldBounds.GetMaxCorner();
    GUIBackground.Params[uniform_minWorldPos].Float().SetValue(Vector2f(min.x, max.y));
    GUIBackground.Params[uniform_maxWorldPos].Float().SetValue(Vector2f(max.x, min.y));
    RenderChild(&GUIBackground, elapsedTime, info);
    
    RoomInfo& rm = GetCurrentRoom();

    //Render each grid square.
    GUIBox.Depth = 0.01f;
    for (unsigned int x = 0; x < rm.RoomGrid.GetWidth(); ++x)
    {
        for (unsigned int y = 0; y < rm.RoomGrid.GetHeight(); ++y)
        {
            BlockTypes thisBlock = rm.RoomGrid[Vector2u(x, y)];
            if (thisBlock == BT_NONE)
            {
                continue;
            }

            Vector4f color;
            switch (rm.RoomGrid[Vector2u(x, y)])
            {
                case BT_WALL:
                    color = Vector4f(0.5f, 0.5f, 0.5f, 0.7f);
                    break;
                case BT_DOORWAY:
                    color = Vector4f(0.0f, 1.0f, 0.0f, 0.7f);
                    break;
                case BT_SPAWN:
                    color = Vector4f(0.0f, 0.0f, 1.0f, 0.7f);
                    break;
                default:
                    assert(false);
                    break;
            }
            RenderBox(Vector2f((float)x + 0.5f, (float)y + 0.5f),
                      Vector2f(0.9f, 0.9f), color, elapsedTime, info);
        }
    }
    
    //Render the grid spot the mouse is touching.
    if (IsValidGridPos(mouseDrawWorldPos))
    {
        Vector4f mouseColor;
        float mouseScale = 0.9f;
        switch (TypeBeingPlaced)
        {
            case BT_NONE:
                mouseColor = Vector4f(0.2f, 0.2f, 0.2f, 0.8f);
                break;
            case BT_WALL:
                mouseColor = Vector4f(0.8f, 0.8f, 0.8f, 0.5f);
                break;
            case BT_DOORWAY:
                mouseColor = Vector4f(0.0f, 1.0f, 0.0f, 0.5f);
                break;
            case BT_SPAWN:
                mouseColor = Vector4f(0.0f, 0.0f, 1.0f, 0.5f);
                break;
            default:
                assert(false);
                break;
        }
        GUIBox.Depth = 0.03f;
        RenderBox(mouseDrawWorldPos + Vector2f(0.5f, 0.5f),
                  Vector2f(mouseScale, mouseScale), mouseColor,
                  elapsedTime, info);
    }
}
void RoomEditorView::RenderBox(Vector2f worldCenter, Vector2f worldSize, Vector4f color,
                               float elapsedTime, const RenderInfo& info)
{
    Vector2f scale = worldSize * 0.5f * viewScale,
             boundsSize = GetBounds().GetDimensions();
    Vector2f oldScale = GUIBox.GetScale();
    Vector2f relPos = GetRelativePos(worldCenter);
    GUIBox.SetPosition(relPos);
    GUIBox.ScaleBy(Vector2f(scale.x, scale.y * boundsSize.x / boundsSize.y));
    GUIBox.SetColor(color);
    RenderChild(&GUIBox, elapsedTime, info);
    GUIBox.SetScale(oldScale);
}

void RoomEditorView::OnMouseClick(Vector2f relativeMousePos)
{
    if (GetBounds().IsPointInside(relativeMousePos))
    {
        lastDragWorldPos = GetWorldPos(relativeMousePos);
        if (GetGridBounds().PointTouches(lastDragWorldPos))
        {
            mouseState = MS_MAKING_BLOCKS;
        }
        else
        {
            mouseState = MS_DRAGGING_CAMERA;
        }
    }
}
void RoomEditorView::OnMouseDrag(Vector2f originalPos, Vector2f newPos)
{
    Vector2f originalWorldPos = GetWorldPos(originalPos),
             newWorldPos = GetWorldPos(newPos),
             deltaPosFrame = newWorldPos - lastDragWorldPos;
    Box2D relativeBounds = GetBounds(),
          worldBounds = GetWorldBounds(relativeBounds);
    
    RoomInfo& rm = GetCurrentRoom();

    switch (mouseState)
    {
        case MS_MAKING_BLOCKS:
            if (IsValidGridPos(mouseDrawWorldPos))
            {
                rm.RoomGrid[ToV2u(mouseDrawWorldPos)] = TypeBeingPlaced;
            }
            break;

        case MS_DRAGGING_CAMERA:
            worldPosCenter -= deltaPosFrame;
            break;

        default:
            return;
    }

    lastDragWorldPos = GetWorldPos(newPos);;
}
void RoomEditorView::OnMouseRelease(Vector2f relativeMousePos)
{
    mouseState = MS_NONE;
}

Vector2f RoomEditorView::GetWorldPos(Vector2f relativePos) const
{
    return worldPosCenter + (relativePos / viewScale);
}
Vector2f RoomEditorView::GetRelativePos(Vector2f worldPos) const
{
    return (worldPos - worldPosCenter) * viewScale;
}

Vector2f RoomEditorView::GetClosestGridPos(Vector2f worldPos) const
{
    return ToV2f((worldPos + Vector2f(0.5f, 0.5f)).Floored());
}

Box2D RoomEditorView::GetWorldBounds(Box2D screenBounds) const
{
    return Box2D(worldPosCenter,
                 screenBounds.GetDimensions() / viewScale);
}

Box2D RoomEditorView::GetGridBounds(void) const
{
    const RoomInfo& rm = GetCurrentRoom();
    return Box2D(0.0f, (float)rm.RoomGrid.GetWidth(),
                 0.0f, (float)rm.RoomGrid.GetHeight());
}

bool RoomEditorView::IsValidGridPos(Vector2f gridPos) const
{
    const RoomInfo& rm = GetCurrentRoom();
    return gridPos.x >= 0.0f && gridPos.y >= 0.0f &&
           gridPos.x < rm.RoomGrid.GetWidth() &&
           gridPos.y < rm.RoomGrid.GetHeight();
}
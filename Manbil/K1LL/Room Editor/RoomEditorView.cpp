#include "RoomEditorView.h"

#include <SFML/Window.hpp>
#include "../../DebugAssist.h"

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

    DataNode::Ptr finalCol(new CustomExpressionNode("min('0' + max('1'.x, '1'.y), 1.0) * '2' * '3'", 4,
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
    matData.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_UV", vIn_UV));

    matData.MaterialOuts.FragmentOutputs.clear();
    matData.MaterialOuts.FragmentOutputs.push_back(ShaderOutput("fOut_Color", colorUniform));

    genM = ShaderGenerator::GenerateMaterial(matData, GUIBox.Params, BlendMode::GetOpaque());
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


    //There always has to be at least one room.
    Rooms.Rooms.push_back(RoomInfo());
    Rooms.Rooms[0].RoomGrid.Resize(5, 5, BT_NONE);
    Rooms.Rooms[0].RoomGrid.Fill(BT_NONE);
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
    GUIBackground.Params.Floats[uniform_minWorldPos].SetValue(Vector2f(min.x, max.y));
    GUIBackground.Params.Floats[uniform_maxWorldPos].SetValue(Vector2f(max.x, min.y));
    RenderChild(&GUIBackground, elapsedTime, info);
    
    RoomInfo& rm = GetCurrentRoom();

    //Render each grid square.
    GUIBox.Depth = -0.01f;
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
                    color = Vector4f(0.5f, 0.5f, 0.5f, 1.0f);
                    break;
                case BT_DOORWAY:
                    color = Vector4f(0.0f, 1.0f, 0.0f, 1.0f);
                    break;
                case BT_ITEM_SPAWN:
                    color = Vector4f(0.0f, 0.0f, 1.0f, 1.0f);
                    break;
                case BT_PLAYER_SPAWN:
                    color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
                    break;
                default:
                    assert(false);
                    break;
            }
            RenderBox(Vector2f((float)x + 0.5f, (float)y + 0.5f),
                      Vector2f(1.0f, 1.0f), color, elapsedTime, info);
        }
    }

    //Render each nav node.
    GUIBox.Depth = -0.02f;
    for (unsigned int i = 0; i < rm.NavNodes.size(); ++i)
    {
        RenderBox(Vector2f((float)rm.NavNodes[i].x, (float)rm.NavNodes[i].y),
                  Vector2f(0.5f, 0.5f), Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
                  elapsedTime, info);
    }
    
    //Render the grid spot the mouse is touching.
    if (IsValidGridPos(mouseDrawWorldPos))
    {
        GUIBox.Depth = -0.03f;
        Vector4f mouseColor;
        float mouseScale;
        switch (PlacingStatus)
        {
            case PS_NODE_ADD:
                mouseScale = 0.5f;
                mouseColor = Vector4f(1.0f, 1.0f, 1.0f, 0.5f);
                break;
            case PS_NODE_REMOVE:
                mouseScale = 0.5f;
                mouseColor = Vector4f(0.1f, 0.1f, 0.1f, 0.5f);
                break;
            case PS_BLOCK:
                mouseScale = 1.0f;
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
                    case BT_ITEM_SPAWN:
                        mouseColor = Vector4f(0.0f, 0.0f, 1.0f, 0.5f);
                        break;
                    case BT_PLAYER_SPAWN:
                        mouseColor = Vector4f(1.0f, 0.0f, 0.0f, 0.5f);
                        break;
                    default:
                        assert(false);
                        break;
                }
                break;
            default:
                assert(false);
                break;
        }
        RenderBox(mouseDrawWorldPos + Vector2f(0.5f, 0.5f),
                  Vector2f(mouseScale, mouseScale), mouseColor,
                  elapsedTime, info);
    }
}
void RoomEditorView::RenderBox(Vector2f worldCenter, Vector2f worldSize, Vector4f color,
                               float elapsedTime, const RenderInfo& info)
{
    Vector2f scale = worldSize * 0.5f * viewScale;
    Vector2f oldScale = GUIBox.GetScale();
    Vector2f relPos = GetRelativePos(worldCenter);
    std::cout << "Rendering box at " << DebugAssist::ToString(worldCenter) << ", size " << DebugAssist::ToString(worldSize) << ", scale: " << std::to_string(scale.x) << ", relative: " << DebugAssist::ToString(relPos) << ", color: " << DebugAssist::ToString(color) << "\n";
    GUIBox.SetPosition(relPos);
    GUIBox.SetRotation(0.0f);
    GUIBox.ScaleBy(scale);
    GUIBox.SetColor(color);
    RenderChild(&GUIBox, elapsedTime, info);
    GUIBox.SetScale(oldScale);
}
void RoomEditorView::RenderLine(Vector2f worldStart, Vector2f worldEnd, float worldThickness,
                                Vector4f color, float elapsedTime, const RenderInfo& info)
{
    Vector2f delta = worldEnd - worldStart;

    GUIBox.SetRotation(atan2f(delta.y, delta.x));
    RenderBox((worldStart + worldEnd) * 0.5f,
              Vector2f(delta.Length(), worldThickness),
              color, elapsedTime, info);
}

void RoomEditorView::OnMouseClick(Vector2f relativeMousePos)
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
            switch (PlacingStatus)
            {
                case PS_NODE_ADD:
                    //Only add a node if this is a valid grid position,
                    //    and a node doesn't exist there yet.
                    if (IsValidGridPos(mouseDrawWorldPos) &&
                        rm.RoomGrid[ToV2u(mouseDrawWorldPos)] != BT_WALL &&
                        !IsNodeAtPos(ToV2u(mouseDrawWorldPos)))
                    {
                        rm.NavNodes.push_back(ToV2u(mouseDrawWorldPos));
                    }
                    break;
                case PS_NODE_REMOVE:
                    //Only add a node if this is a valid grid position, and a node exists there already.
                    if (IsValidGridPos(mouseDrawWorldPos) && IsNodeAtPos(ToV2u(mouseDrawWorldPos)))
                    {
                        rm.NavNodes.erase(std::find(rm.NavNodes.begin(),
                                                          rm.NavNodes.end(),
                                                          ToV2u(mouseDrawWorldPos)));
                    }
                    break;
                case PS_BLOCK:
                    //Only add a block if this is a valid grid position,
                    //    and we're not going to block a node.
                    if (IsValidGridPos(mouseDrawWorldPos) &&
                        (TypeBeingPlaced != BT_WALL || !IsNodeAtPos(ToV2u(mouseDrawWorldPos))))
                    {
                        rm.RoomGrid[ToV2u(mouseDrawWorldPos)] = TypeBeingPlaced;
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
            break;

        case MS_DRAGGING_CAMERA:
            worldPosCenter -= deltaPosFrame;
            break;

        default:
            assert(false);
            break;
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
bool RoomEditorView::IsNodeAtPos(Vector2u gridPos) const
{
    const RoomInfo& rm = GetCurrentRoom();
    return std::find(rm.NavNodes.begin(), rm.NavNodes.end(), gridPos) !=
           rm.NavNodes.end();
}
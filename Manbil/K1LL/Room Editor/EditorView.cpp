#include "EditorView.h"

#include "../../Rendering/Data Nodes/DataNodes.hpp"
#include "../../Rendering/Data Nodes/ShaderGenerator.h"

#include "../../Rendering/GUI/GUIMaterials.h"

#include <SFML/Window.hpp>


const float ZoomSpeed = 1.1f;
const std::string uniform_minWorldPos = "u_minWorldPos",
                  uniform_maxWorldPos = "u_maxWorldPos";
const std::string uniform_lineStart = "u_lineStart",
                  uniform_lineEnd = "u_lineEnd",
                  uniform_lineThickness = "u_thickness";

Vector2f RoundToInt(Vector2f inV)
{
    return Vector2f((float)Mathf::RoundToInt(inV.x),
                    (float)Mathf::RoundToInt(inV.y));
}


EditorView::EditorView(std::string& err)
    : viewScale(50.0f), worldPosCenter(5.0f, 5.0f), MouseWheelDelta(0),
      backgroundTex(TextureSampleSettings2D(FT_NEAREST, WT_WRAP), PS_8U, false),
      GUIElement(UniformDictionary())
{
    RenderIOAttributes quadVertIns = DrawingQuad::GetVertexInputData();

    SerializedMaterial matData(quadVertIns);

    DataLine vIn_Pos(VertexInputNode::GetInstance(), 0),
             vIn_UV(VertexInputNode::GetInstance(), 1);

    DataNode::Ptr minWorldPos(new ParamNode(2, uniform_minWorldPos)),
                  maxWorldPos(new ParamNode(2, uniform_maxWorldPos));

    DataNode::Ptr objToScreen = SpaceConverterNode::ObjPosToScreenPos(vIn_Pos, "objPosToScreen");
    DataLine screenPos4(objToScreen, 1);

    DataNode::Ptr objToWorld(new CustomExpressionNode("mix('0', '1', 0.5 + (0.5 * '2'.xy))",
                                                      2, minWorldPos, maxWorldPos, vIn_Pos,
                                                      "objPosToWorld"));

    DataNode::Ptr vOut_UV(new CustomExpressionNode("0.5 + (0.5 * '0'.xy)", 2, vIn_Pos, "vOut_UV"));

    matData.MaterialOuts.VertexPosOutput = screenPos4;
    matData.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_WorldPos", objToWorld));
    matData.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_ObjPos", vOut_UV));

    DataLine fIn_WorldPos(FragmentInputNode::GetInstance(), 0),
             fIn_UV(FragmentInputNode::GetInstance(), 1);

    DataNode::Ptr distToOrigin(new DistanceNode(fIn_WorldPos, Vector2f(0.0f, 0.0f), "distToOrigin")),
                  originStrength(new CustomExpressionNode("max(0.0, pow(1.0 - (min('0', 3.0) / 3.0), 3.5))",
                                                          1, distToOrigin, "originStrength"));
    DataNode::Ptr gridStrength(new CustomExpressionNode("pow(2.0 * abs(fract(abs('0')) - 0.5), vec2(6.5))",
                                                        2, fIn_WorldPos, "gridStrength"));

    DataNode::Ptr colorUniform(new ParamNode(4, GUIMaterials::QuadDraw_Color));

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
    backgroundMat = genM.Mat;
    GUIBackground.Mat = backgroundMat;

    backgroundTex.Create();
    if (!backgroundTex.SetDataFromFile("Content/Room Editor/Background.png", err))
    {
        err = "Error loading background tex: " + err;
        return;
    }
    GUIBackground.SetTex(&backgroundTex);
    GUIBackground.SetColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));



    DataNode::Ptr lineStart(new ParamNode(2, uniform_lineStart)),
                  lineEnd(new ParamNode(2, uniform_lineEnd)),
                  lineThickness(new ParamNode(1, uniform_lineThickness));

    std::string posExpr =
"mix('0', '1', 0.5 + (0.5 * '2'.x)) + \n\
(('1' - '0').yx * '2'.y * '3')";
    DataNode::Ptr linePos(new CustomExpressionNode(posExpr, 2,
                                                   lineStart, lineEnd, vIn_Pos, lineThickness,
                                                   "linePos"));
    DataNode::Ptr linePos3(new CombineVectorNode(linePos, 0.0f, "linePos3"));

    objToScreen = SpaceConverterNode::ObjPosToScreenPos(linePos3, "linePosToScreen");

    finalCol = DataNode::Ptr(new CustomExpressionNode("min(max('0'.x, '0'.y), 1.0) * '1'", 4,
                                                      gridStrength, colorUniform, "finalLineCol"));

    matData.MaterialOuts.VertexPosOutput = DataLine(objToScreen, 1);
    matData.MaterialOuts.VertexOutputs.erase(matData.MaterialOuts.VertexOutputs.end() - 1);
    matData.MaterialOuts.VertexOutputs[0].Value = linePos;
    matData.MaterialOuts.FragmentOutputs[0].Value = finalCol;

    genM = ShaderGenerator::GenerateMaterial(matData, lineMatParams, BlendMode::GetOpaque());
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating line material: " + genM.ErrorMessage;
        return;
    }
    lineMat = genM.Mat;

    lineMatParams.Floats[uniform_lineThickness].SetValue(0.01f);
}

EditorView::~EditorView(void)
{
    if (backgroundMat != 0)
    {
        delete backgroundMat;
    }
    if (lineMat != 0)
    {
        delete lineMat;
    }
}

Box2D EditorView::GetBounds(void) const
{
    Box2D bnds = GUIBackground.GetBounds();
    bnds.Move(GUIBackground.GetPos());
    return bnds;
}


void EditorView::ScaleBy(Vector2f scaleAmount)
{
    GUIBackground.ScaleBy(scaleAmount);
    GUIElement::ScaleBy(scaleAmount);
}
void EditorView::SetScale(Vector2f newScale)
{
    GUIBackground.SetScale(newScale);
    GUIElement::SetScale(newScale);
}

#include "../../DebugAssist.h"
void EditorView::CustomUpdate(float elapsed, Vector2f mousePos)
{
    if (MouseWheelDelta != 0)
    {
        viewScale *= powf(ZoomSpeed, (float)MouseWheelDelta);
        MouseWheelDelta = 0;
    }
    std::cout << DebugAssist::ToString(GetWorldPos(mousePos)) << "\n";
}

void EditorView::Render(float elapsedTime, const RenderInfo& info)
{
    //Render the background.
    Box2D worldBounds = GetWorldBounds(GetBounds());
    Vector2f min = worldBounds.GetMinCorner(),
             max = worldBounds.GetMaxCorner();
    GUIBackground.Params.Floats[uniform_minWorldPos].SetValue(Vector2f(min.x, max.y));
    GUIBackground.Params.Floats[uniform_maxWorldPos].SetValue(Vector2f(max.x, min.y));
    RenderChild(&GUIBackground, elapsedTime, info);

    //Render the grid spot the mouse is touching.
    //TODO: Implement. Maybe just render a "line" that spans a tiny space across the grid spot?

    //Render each line.
    for (unsigned int i = 0; i < RoomData.Walls.size(); ++i)
    {
        RenderLine(RoomData.Walls[i].Start, RoomData.Walls[i].End,
                   Vector4f(1.0f, 1.0f, 1.0f, 1.0f),
                   elapsedTime, info);
    }
}
void EditorView::RenderLine(Vector2f worldStart, Vector2f worldEnd, Vector4f color,
                            float elapsedSeconds, const RenderInfo& info)
{
    lineMatParams.Floats[uniform_lineStart].SetValue(GetRelativePos(worldStart) + GetPos());
    lineMatParams.Floats[uniform_lineEnd].SetValue(GetRelativePos(worldEnd) + GetPos());
    lineMatParams.Floats[GUIMaterials::QuadDraw_Color].SetValue(color);
    DrawingQuad::GetInstance()->Render(info, lineMatParams, *lineMat);
}

void EditorView::OnMouseClick(Vector2f relativeMousePos)
{
    mouseState = MS_CLICKING;
}
void EditorView::OnMouseDrag(Vector2f originalPos, Vector2f newPos)
{
    Vector2f originalWorldPos = GetWorldPos(originalPos),
             newWorldPos = GetWorldPos(newPos);
    Box2D relativeBounds = GetBounds(),
          worldBounds = GetWorldBounds(relativeBounds);

    switch (mouseState)
    {
        case MS_CLICKING:
            if (originalPos.Distance(newPos) > 2.0f)
            {
                Vector2f closestGrid = GetClosestGridPos(originalWorldPos);

                bool clickingVertex = false;
                for (unsigned int i = 0; i < RoomData.Walls.size(); ++i)
                {
                    if (RoundToInt(RoomData.Walls[i].Start) == closestGrid)
                    {
                        clickingVertex = true;
                        vertexBeingDragged = Vertex(i, false);
                        break;
                    }
                    else if (RoundToInt(RoomData.Walls[i].End) == closestGrid)
                    {
                        clickingVertex = true;
                        vertexBeingDragged = Vertex(i, true);
                        break;
                    }
                }

                mouseState = (clickingVertex ? MS_DRAGGING_VERTEX : MS_DRAGGING_CAMERA);
            }
            break;

        case MS_DRAGGING_CAMERA:
            //TODO: Move the camera.
            break;

        case MS_DRAGGING_VERTEX:
            //TODO: Move the vertex.
            break;
    }
}
void EditorView::OnMouseRelease(Vector2f relativeMousePos)
{
    switch (mouseState)
    {
        case MS_CLICKING:
            break;

        case MS_DRAGGING_CAMERA:
            break;

        case MS_DRAGGING_VERTEX:
            break;

        default: assert(false);
    }
    mouseState = MS_NONE;
}


Vector2f EditorView::GetWorldPos(Vector2f relativePos) const
{
    return worldPosCenter + (relativePos / viewScale);
}
Vector2f EditorView::GetRelativePos(Vector2f worldPos) const
{
    return (worldPos - worldPosCenter) * viewScale;
}

Vector2f EditorView::GetClosestGridPos(Vector2f worldPos) const
{
    Vector2f closestGrid = ToV2f(worldPos.Floored());

    Vector2f dist = (closestGrid - worldPos).Abs();
    if (dist.x > 0.5f)
    {
        closestGrid.x += 2.0f * (worldPos.x - closestGrid.x);
    }
    if (dist.y > 0.5f)
    {
        closestGrid.y += 2.0f * (worldPos.y - closestGrid.y);
    }

    return RoundToInt(closestGrid);
}

Box2D EditorView::GetWorldBounds(Box2D screenBounds) const
{
    return Box2D(worldPosCenter,
                 screenBounds.GetDimensions() / viewScale);
}
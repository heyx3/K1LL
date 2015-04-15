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


EditorView::EditorView(std::string& err)
    : viewScale(50.0f), worldPosCenter(5.0f, 5.0f),
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

    DataNode::Ptr objToWorld(new CustomExpressionNode("mix('0', '1', 0.5 + (0.5 * '2'))",
                                                      2, minWorldPos, maxWorldPos, vIn_Pos,
                                                      "objPosToWorld"));

    matData.MaterialOuts.VertexPosOutput = screenPos4;
    matData.MaterialOuts.VertexOutputs.push_back(ShaderOutput("fIn_WorldPos", objToWorld));

    DataLine fIn_WorldPos(FragmentInputNode::GetInstance(), 0);

    DataNode::Ptr distToOrigin(new DistanceNode(fIn_WorldPos, Vector2f(0.0f, 0.0f), "distToOrigin")),
                  originStrength(new CustomExpressionNode("pow(1.0 - (min('0', 3.0) / 3.0), 3.5)",
                                                          1, distToOrigin, "originStrength"));
    DataNode::Ptr gridStrength(new CustomExpressionNode("pow(2.0 * abs(fract(abs('0')) - 0.5), vec2(6.5)",
                                                        2, fIn_WorldPos, "gridStrength"));

    DataNode::Ptr colorUniform(new ParamNode(4, GUIMaterials::QuadDraw_Color));

    DataNode::Ptr finalCol(new CustomExpressionNode("min('0' + max('1'.x, '1'.y), 1.0) * '2'", 4,
                                                    originStrength, gridStrength, colorUniform,
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



    DataNode::Ptr lineStart(new ParamNode(2, uniform_lineStart)),
                  lineEnd(new ParamNode(2, uniform_lineEnd)),
                  lineThickness(new ParamNode(1, uniform_lineThickness));

    std::string posExpr =
"mix('0', '1', 0.5 + (0.5 * '2'.x)) + \n\
(('1' - '0').yx * '2'.y * '3')";
    DataNode::Ptr linePos(new CustomExpressionNode(posExpr, 2,
                                                   lineStart, lineEnd, vIn_Pos, lineThickness,
                                                   "linePos"));
    DataNode::Ptr linePos4(new CombineVectorNode(linePos, 0.0f, 1.0f, "linePos4"));

    objToScreen = SpaceConverterNode::ObjPosToScreenPos(linePos4, "linePosToScreen");

    matData.MaterialOuts.VertexPosOutput = DataLine(objToScreen, 1);
    matData.MaterialOuts.VertexOutputs.clear();
    matData.MaterialOuts.FragmentOutputs[0].Value = finalCol;

    genM = ShaderGenerator::GenerateMaterial(matData, lineMatParams, BlendMode::GetOpaque());
    if (!genM.ErrorMessage.empty())
    {
        err = "Error generating line material: " + genM.ErrorMessage;
        return;
    }
    lineMat = genM.Mat;
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
    Vector2f scale = GetScale();
    return Box2D(-scale.x, scale.x, -scale.y, scale.y);
}

#include "../../DebugAssist.h"
void EditorView::CustomUpdate(float elapsed, Vector2f mousePos)
{
    if (MouseWheelDelta != 0)
    {
        viewScale *= ZoomSpeed * (float)MouseWheelDelta;
        MouseWheelDelta = 0;
    }
    std::cout << DebugAssist::ToString(GetWorldPos(mousePos)) << "\n";
}
void EditorView::Render(float elapsedTime, const RenderInfo& info)
{
    //Render the background.
    Box2D worldBounds = GetWorldBounds(GetBounds());
    GUIBackground.Params.Floats[uniform_minWorldPos].SetValue(worldBounds.GetMinCorner());
    GUIBackground.Params.Floats[uniform_maxWorldPos].SetValue(worldBounds.GetMaxCorner());
    RenderChild(&GUIBackground, elapsedTime, info);

    //Render each line.
    for (unsigned int i = 0; i < RoomData.Walls.size(); ++i)
    {
        RenderLine(RoomData.Walls[i].Start, RoomData.Walls[i].End, elapsedTime, info);
    }
}
void EditorView::RenderLine(Vector2f worldStart, Vector2f worldEnd,
                            float elapsedSeconds, const RenderInfo& info)
{
    lineMatParams.Floats[uniform_lineStart].SetValue(GetRelativePos(worldStart) + GetPos());
    lineMatParams.Floats[uniform_lineEnd].SetValue(GetRelativePos(worldEnd) + GetPos());
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
                
               // if (originalWorldPos.Abs())
            }
            break;

        case MS_DRAGGING_CAMERA:

            break;

        case MS_DRAGGING_VERTEX:
            
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
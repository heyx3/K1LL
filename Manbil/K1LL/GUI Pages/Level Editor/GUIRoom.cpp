#include "GUIRoom.h"

#include "../../Content/MenuContent.h"
#include "LevelEditor.h"
#include "../PageManager.h"
#include "GUIRoom.h"


#define MC MenuContent::Instance


//Color and depth constants.
namespace
{
    const Vector4f minBlendCol = Vector4f(0.6f, 0.6f, 0.6f, 0.75f),
                   maxBlendCol = Vector4f(0.8f, 0.8f, 1.0f, 0.95f);
    const float blendSpeed = 3.0f;

    const Vector3f COLOR_Wall(0.8f, 0.8f, 0.8f),
                   COLOR_Spawn(1.0f, 1.0f, 1.0f);

    const float DEPTH_BlockOffset = 0.1f;
}


GUIRoom::GUIRoom(const GUIEditorGrid& grid, LevelEditor& editor, bool isBeingPlaced,
                 const LevelInfo::RoomData* roomData)
    : Grid(grid), Editor(editor), IsBeingPlaced(isBeingPlaced),
      RoomData(roomData),
      GUITexture(MC.StaticColorGUIParams, &MC.FloorTex, MC.StaticColorGUIMat)
{

}


void GUIRoom::Render(float seconds, const RenderInfo& info)
{
    Vector4f baseCol = GetColor();

    //Set this room's overall color.
    if (IsBeingPlaced)
    {
        float lerpT = 0.5f + (0.5f * sinf(Editor.Manager->GetTotalElapsedSeconds() * blendSpeed));
        SetColor(baseCol.ComponentProduct(Vector4f::Lerp(minBlendCol, maxBlendCol, lerpT)));
    }

    //Render the floor.
    GUITexture::Render(seconds, info);


    //Render the walls.

    Box2D roomBounds_Screen = GetBounds();
    roomBounds_Screen.Move(GetPos());
    
    Vector2f roomMin_Screen = roomBounds_Screen.GetMinCorner(),
             roomSize_Screen = roomBounds_Screen.GetDimensions(),
             blockSize_Screen(roomSize_Screen.x / (float)RoomData->Walls.GetWidth(),
                              roomSize_Screen.y / (float)RoomData->Walls.GetHeight());

    GUITexture child(Params, &MC.WallTex, Mat);
    child.Depth = Depth + DEPTH_BlockOffset;

    for (Vector2u gridLocal; gridLocal.y < RoomData->Walls.GetHeight(); ++gridLocal.y)
    {
        for (gridLocal.x = 0; gridLocal.x < RoomData->Walls.GetWidth(); ++gridLocal.x)
        {
            BlockTypes block = RoomData->Walls[gridLocal];
            
            //Only draw walls/doorways.
            if (block == BT_NONE || block == BT_DOORWAY)
            {
                continue;
            }
            else if (block == BT_WALL)
            {
                child.SetTex(&MC.WallTex);
                child.SetColor(Vector4f(COLOR_Wall, 1.0f).ComponentProduct(GetColor()));
                
                //Position the block.
                Vector2f blockMin_Screen = roomMin_Screen +
                                           ToV2f(gridLocal).ComponentProduct(blockSize_Screen);
                child.SetBounds(Box2D(blockMin_Screen.x, blockMin_Screen.y, blockSize_Screen));

                child.Render(seconds, info);
            }
            else if (block == BT_SPAWN)
            {
                MTexture2D* itemTex = MC.GetPickupTex(RoomData->SpawnedItem);
                if (itemTex != 0)
                {
                    child.SetTex(itemTex);

                    //Set the color and oscillate the alpha.
                    float t = Editor.Manager->GetTotalElapsedSeconds();
                    float alpha = Mathf::Lerp(0.4f, 0.8f, 0.2f + (0.2f * sinf(4.0f * t)));
                    child.SetColor(GetColor().ComponentProduct(Vector4f(COLOR_Spawn, alpha)));

                    //Position the block.
                    Vector2f blockMin_Screen = roomMin_Screen +
                                               ToV2f(gridLocal).ComponentProduct(blockSize_Screen);
                    child.SetBounds(Box2D(blockMin_Screen.x, blockMin_Screen.y, blockSize_Screen));

                    child.Render(seconds, info);
                }
            }
            else
            {
                //Unknown block type!
                assert(false);
            }
        }
    }
}

#undef MC
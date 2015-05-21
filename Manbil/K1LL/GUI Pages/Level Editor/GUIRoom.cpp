#include "GUIRoom.h"

#include "../../Content/MenuContent.h"
#include "LevelEditor.h"
#include "../PageManager.h"
#include "GUIRoom.h"


#define MC MenuContent::Instance


//If placing the room, oscillate its color between two values.
namespace
{
    const Vector4f minBlendCol = Vector4f(0.6f, 0.6f, 0.6f, 0.75f),
                   maxBlendCol = Vector4f(0.8f, 0.8f, 1.0f, 0.95f);
    const float blendSpeed = 3.0f;
}


GUIRoom::GUIRoom(const GUIEditorGrid& grid, const LevelEditor& editor, bool isBeingPlaced,
                 const RoomInfo* roomData, ItemTypes roomSpawn)
    : Grid(grid), Editor(editor), IsBeingPlaced(isBeingPlaced),
      RoomData(roomData), RoomSpawn(roomSpawn),
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
             blockSize_Screen(roomSize_Screen.x / (float)RoomData->RoomGrid.GetWidth(),
                              roomSize_Screen.y / (float)RoomData->RoomGrid.GetHeight());

    GUITexture child(Params, &MC.WallTex, Mat);
    child.Depth = Depth + 0.001f;

    for (Vector2u gridLocal; gridLocal.y < RoomData->RoomGrid.GetHeight(); ++gridLocal.y)
    {
        for (gridLocal.x = 0; gridLocal.x < RoomData->RoomGrid.GetWidth(); ++gridLocal.x)
        {
            BlockTypes block = RoomData->RoomGrid[gridLocal];
            
            //Only draw walls/doorways.
            if (block == BT_NONE || block == BT_SPAWN)
            {
                continue;
            }
            else if (block == BT_WALL || block == BT_DOORWAY)
            {
                child.SetTex(&MC.WallTex);

                //Draw doorways as partially-transparent walls.
                Vector4f color = block == BT_WALL ?
                                     Vector4f(1.0f, 1.0f, 1.0f, 1.0f) :
                                     Vector4f(1.0f, 1.0f, 1.0f, 0.25f);
                child.SetColor(color.ComponentProduct(GetColor()));
                
                //Position the block.
                Vector2f blockMin_Screen = roomMin_Screen +
                                           ToV2f(gridLocal).ComponentProduct(blockSize_Screen);
                child.SetBounds(Box2D(blockMin_Screen.x, blockMin_Screen.y, blockSize_Screen));

                child.Render(seconds, info);
            }
            else if (block == BT_SPAWN)
            {
                MTexture2D* itemTex = MC.GetPickupTex(RoomSpawn);
                if (itemTex != 0)
                {
                    child.SetTex(itemTex);

                    float alpha = Mathf::Lerp(0.4f, 0.8f,
                                              0.5f + (0.5f *
                                                      sinf(0.8f * Editor.Manager->GetTotalElapsedSeconds())));
                    child.SetColor(Vector4f(1.0f, 1.0f, 1.0f, 0.4f));

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
#include "GUIRoom.h"

#include "../../Content/MenuContent.h"
#include "LevelEditor.h"
#include "../PageManager.h"
#include "GUIRoom.h"


#define MC MenuContent::Instance


//If placing the room, oscillate its color between two values.
namespace
{
    const Vector4f minBlendCol = Vector4f(0.0f, 0.0f, 0.6f, 0.5f),
                   maxBlendCol = Vector4f(0.6f, 0.6f, 1.0f, 0.75f);
    const float blendSpeed = 1.0f;
}


GUIRoom::GUIRoom(const GUIEditorGrid& grid, const LevelEditor& editor, bool isBeingPlaced,
                 const RoomInfo* roomData, ItemTypes roomSpawn)
    : Grid(grid), Editor(editor), IsBeingPlaced(isBeingPlaced),
      RoomData(roomData), RoomSpawn(roomSpawn),
      GUITexture(MC.StaticColorGUIParams, &MC.FloorTex, MC.StaticColorGUIMat)
{

}

/*
Vector2f GUIRoom::WorldPosToScreen(Vector2f worldPos) const
{
    Vector2f worldViewMin = Grid.WorldViewBounds.GetMinCorner(),
             worldViewSize = Grid.WorldViewBounds.GetDimensions();
    return Vector2f::Lerp(Vector2f(0.0f, -(float)Editor.Manager->GetWindowSize().y),
                          Vector2f((float)Editor.Manager->GetWindowSize().x, 0.0f),
                          Vector2f((worldPos.x - worldViewMin.x) / worldViewSize.x,
                                   (worldPos.y - worldViewMin.y) / worldViewSize.y));
}
Vector2f GUIRoom::WorldSizeToScreen(Vector2f worldSize) const
{
    Vector2f sizeLerp(worldSize.x / Grid.WorldViewBounds.GetXSize(),
                      worldSize.y / Grid.WorldViewBounds.GetYSize());
    return sizeLerp.ComponentProduct(ToV2f(Editor.Manager->GetWindowSize()));
}
*/

/*
Box2D GUIRoom::GetBounds(void) const
{
    Vector2f worldViewDims = WorldViewBounds.GetDimensions(),
             worldViewMin = WorldViewBounds.GetMinCorner(),
             worldViewMax = WorldViewBounds.GetMaxCorner();

    Vector2f windowSize = ScreenDrawSize;

    Vector2f worldRoomDims = ToV2f(RoomData->RoomGrid.GetDimensions());

    Vector2f screenRoomDims((worldRoomDims.x - worldViewMin.x) / worldViewDims.x,
                            (worldRoomDims.y - worldViewMin.y) / worldViewDims.y);
    screenRoomDims = Vector2f::Lerp(Vector2f(0.0f, -windowSize.y),
                                    Vector2f(windowSize.x, 0.0f),
                                    screenRoomDims);

    Vector2f worldRoomCenter = ToV2f(RoomGridOffset) + (worldRoomDims * 0.5f);
    Vector2f screenRoomCenter((worldRoomCenter.x - worldViewMin.x) / worldViewDims.x,
                              (worldRoomCenter.y - worldViewMin.y) / worldViewDims.y);
    screenRoomCenter = Vector2f::Lerp(Vector2f(0.0f, -windowSize.x),
                                      Vector2f(windowSize.x, 0.0f),
                                      screenRoomCenter);

    return Box2D(screenRoomCenter - GetPos(), screenRoomDims);
}
*/

void GUIRoom::Render(float seconds, const RenderInfo& info)
{
    //Position the floor so that it covers the room's extent.
    //Vector2f roomSize_World = ToV2f(RoomData->RoomGrid.GetDimensions()),
    //         roomCenter_World = ToV2f(RoomGridOffset) + (roomSize_World * 0.5f);
    //Vector2f roomSize_Screen = WorldSizeToScreen(roomSize_World),
    //         roomCenter_Screen = WorldPosToScreen(roomCenter_World);
    //SetBounds(Box2D(roomCenter_Screen, roomSize_Screen));

    //Set this room's overall color.
    if (IsBeingPlaced)
    {
        float lerpT = 0.5f * (0.5f * sinf(Editor.Manager->GetTotalElapsedSeconds() * blendSpeed));
        SetColor(Vector4f::Lerp(minBlendCol, maxBlendCol, lerpT));
    }
    else
    {
        SetColor(Vector4f(1.0f, 1.0f, 1.0f, 1.0f));
    }

    //Render the floor.
    GUITexture::Render(seconds, info);

    //Reset position/scale so it doesn't affect children.
    //SetPosition(Vector2f());
    //SetScale(Vector2f(1.0f, 1.0f));


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
                //Draw doorways as partially-transparent walls.
                Vector4f color = block == BT_WALL ?
                                     Vector4f(1.0f, 1.0f, 1.0f, 1.0f) :
                                     Vector4f(1.0f, 1.0f, 1.0f, 0.5f);
                child.SetColor(color.ComponentProduct(GetColor()));
                
                //Position the block.
                Vector2f blockMin_Screen = roomMin_Screen +
                                           ToV2f(gridLocal).ComponentProduct(blockSize_Screen);
                child.SetBounds(Box2D(blockMin_Screen.x, blockMin_Screen.y, blockSize_Screen));

                //RenderChild(&child, seconds, info);
                child.Render(seconds, info);
            }
            else
            {
                //Unknown block type!
                assert(false);
            }
        }
    }


    //Render the item that spawns in the room.
    MTexture2D* itemTex = 0;
    switch (RoomSpawn)
    {
        case IT_NONE:
            break;

        case IT_AMMO_LIGHT:
            itemTex = &MC.AmmoLightTex;
            break;
        case IT_AMMO_HEAVY:
            itemTex = &MC.AmmoHeavyTex;
            break;
        case IT_AMMO_SPECIAL:
            itemTex = &MC.AmmoSpecialTex;
            break;

        case IT_WEAPON_LIGHT:
            itemTex = &MC.WeaponLightTex;
            break;
        case IT_WEAPON_HEAVY:
            itemTex = &MC.WeaponHeavyTex;
            break;
        case IT_WEAPON_SPECIAL:
            itemTex = &MC.WeaponSpecialTex;
            break;

        case IT_HEALTH:
            itemTex = &MC.HealthTex;
            break;

        default:
            assert(false);
            break;
    }
    if (itemTex != 0)
    {
        child.SetTex(itemTex);
        child.SetColor(Vector4f(1.0f, 1.0f, 1.0f, 0.4f));

        Box2D childBounds = roomBounds_Screen;
        childBounds.Scale(Vector2f(0.8f, 0.8f));
        child.SetBounds(childBounds);

        child.Render(seconds, info);
    }
}

#undef MC
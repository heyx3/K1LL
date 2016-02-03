#include "PlayerViewport.h"

#include "../../Content/MenuContent.h"
#include "../../Content/Settings.h"
#include "../../Content/PostProcessing.h"


PlayerViewport::PlayerViewport(Level& lvl, Player* target, SFMLWorld* world, std::string& err)
    : Lvl(lvl), Target(target), World(world),
      worldRendColor(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PS_32F, false),
      worldRendDepth(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PS_24U_DEPTH, false),
      worldRendTarg(PS_16U_DEPTH, err),
      GUITexture(MenuContent::Instance.StaticOpaqueColorParams,
                 &worldRendColor, MenuContent::Instance.StaticOpaqueColorMat)
{
    if (!err.empty())
    {
        return;
    }


    worldRendColor.Create();
    worldRendColor.ClearData(world->GetWindow()->getSize().x,
                             world->GetWindow()->getSize().y);

    worldRendDepth.Create();
    
    const RenderTarget* old = RenderTarget::GetCurrentTarget();
    if (!worldRendTarg.SetDepthAttachment(RenderTargetTex(&worldRendDepth)) ||
        !worldRendTarg.SetColorAttachment(RenderTargetTex(&worldRendColor), true))
    {
        err = "Unable to set depth/color attachments for world render target";
        return;
    }
    worldRendTarg.DisableDrawingInto();
    if (old != 0)
    {
        old->EnableDrawingInto();
    }
}


void PlayerViewport::SetScale(Vector2f newScale)
{
    //Instead of scaling this item, scale up the texture itself.
    Vector2f targetSize = newScale.ComponentProduct(ToV2f(Vector2u(worldRendColor.GetWidth(),
                                                                   worldRendColor.GetHeight())));
    Vector2u sizeU = ToV2u(targetSize.Ceil());
    worldRendColor.ClearData(sizeU.x, sizeU.y);
    worldRendTarg.UpdateSize();

    DidBoundsChange = true;
}
void PlayerViewport::ScaleBy(Vector2f scaleAmount)
{
    SetScale(GetScale().ComponentProduct(scaleAmount));
}

void PlayerViewport::Render(float frameSeconds, const RenderInfo& screenRenderInfo)
{
    const RenderTarget* current = RenderTarget::GetCurrentTarget();

    //Render the world.
    worldRendTarg.EnableDrawingInto();

    //TODO: Fix culling.
    RenderingState(RenderingState::C_NONE).EnableState();
    ScreenClearer(true, true, false, Vector4f(1.0f, 0.0f, 1.0f, 0.0f)).ClearScreen();
    
    Vector3f camPos = LevelConstants::Instance.GetPlayerEyePos(Target->Pos, Target->LookDir);
    Camera cam(camPos, Target->LookDir, Vector3f(0.0f, 0.0f, 1.0f), false);
    cam.PerspectiveInfo.SetFOVDegrees(Settings::Instance.FOVDegrees);
    cam.PerspectiveInfo.Width = (float)worldRendTarg.GetWidth();
    cam.PerspectiveInfo.Height = (float)worldRendTarg.GetHeight();
    cam.PerspectiveInfo.zNear = LevelConstants::Instance.CameraZNear;
    cam.PerspectiveInfo.zFar = LevelConstants::Instance.CameraZFar;

    Matrix4f viewM, projM;
    cam.GetViewTransform(viewM);
    cam.GetPerspectiveProjection(projM);

    RenderInfo worldRenderInfo(Lvl.GetTimeSinceGameStart(), &cam, &viewM, &projM);
    Lvl.Render(frameSeconds, worldRenderInfo);

    worldRendTarg.DisableDrawingInto();


    //Render post-processing.
    RenderTarget* finalRnd = PostProcessing::Instance.Apply(worldRendColor, worldRendDepth);

    
    if (current != 0)
    {
        current->EnableDrawingInto();
    }
    else
    {
        Viewport(0, 0,
                 World->GetWindow()->getSize().x,
                 World->GetWindow()->getSize().y).Use();
    }


    //Do the final render to the screen.
    RenderingState(RenderingState::C_NONE).EnableState();
    SetTex(finalRnd->GetColorTextures()[0].MTex);
    GUITexture::Render(frameSeconds, screenRenderInfo);
}
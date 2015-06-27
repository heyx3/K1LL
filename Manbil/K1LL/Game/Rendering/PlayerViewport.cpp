#include "PlayerViewport.h"

#include "../../Content/MenuContent.h"
#include "../../Content/Settings.h"


PlayerViewport::PlayerViewport(Level& lvl, Player* target, SFMLWorld* world, std::string& err)
    : Lvl(lvl), Target(target), World(world),
      worldRendColor(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PS_32F, false),
      worldRendDepth(TextureSampleSettings2D(FT_LINEAR, WT_CLAMP), PS_32F_DEPTH, false),
      worldRendTarg(PS_16U_DEPTH, err),
      GUITexture(MenuContent::Instance.StaticColorGUIParams,
                 &worldRendColor, MenuContent::Instance.StaticColorGUIMat)
{
    if (!err.empty())
    {
        return;
    }


    worldRendColor.Create();
    worldRendColor.ClearData(world->GetWindow()->getSize().x,
                             world->GetWindow()->getSize().y);

    worldRendDepth.Create();
    
    if (!worldRendTarg.SetDepthAttachment(RenderTargetTex(&worldRendDepth)) ||
        !worldRendTarg.SetColorAttachment(RenderTargetTex(&worldRendColor), true))
    {
        err = "Unable to set depth/color attachments for world render target";
        return;
    }

    if (RenderTarget::GetCurrentTarget() != 0)
    {
        RenderTarget::GetCurrentTarget()->DisableDrawingInto(world->GetWindow()->getSize().x,
                                                             world->GetWindow()->getSize().y);
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

    //TODO: Notify the post-process effect of the change.

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

    RenderingState(RenderingState::C_NONE).EnableState();
    ScreenClearer(true, true, false, Vector4f(1.0f, 0.0f, 1.0f, 0.0f)).ClearScreen();
    glViewport(0, 0, worldRendTarg.GetWidth(), worldRendTarg.GetHeight());
    
    Vector3f camPos(Target->Pos, LevelConstants::Instance.PlayerEyeHeight);
    camPos += Vector3f(Target->LookDir.XY().Normalized() * LevelConstants::Instance.PlayerEyeForward, 0.0f);
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

    worldRendTarg.DisableDrawingInto(World->GetWindow()->getSize().x, World->GetWindow()->getSize().y);


    //TODO: Pass world render tex into Post-processing system.


    //Render the final render target to the screen.
    SetTex(&worldRendColor); // TODO: use whatever texture the post-processing system says to use.
    if (current != 0)
    {
        current->EnableDrawingInto();
    }
    GUITexture::Render(frameSeconds, screenRenderInfo);
}
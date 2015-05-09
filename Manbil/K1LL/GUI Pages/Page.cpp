#include "Page.h"

#include <iostream>
#include "../../Rendering/Basic Rendering/ScreenClearer.h"
#include "../../Rendering/Basic Rendering/RenderingState.h"

#include "PageManager.h"


bool Page::Assert(bool test, const std::string& errIntro, const std::string& err)
{
    if (!test)
    {
        std::cout << errIntro << ": " << err << "\n\n\nEnter anything to continue:\n";
        char dummy;
        std::cin >> dummy;

        Manager->EndWorld();
    }
    return test;
}

void Page::Update(Vector2i mousePos, float frameSeconds)
{
    GUIManager.Update(frameSeconds, mousePos,
                      GetIsWindowInFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left));
}
void Page::Render(float frameSeconds)
{
    ScreenClearer(true, true, false, Vector4f(0.02f, 0.02f, 0.15f, 0.0f)).ClearScreen();
    RenderingState(RenderingState::C_NONE).EnableState();

    Vector2f windowSize = ToV2f(Manager->GetWindowSize());

    Camera guiCam;
    guiCam.SetPosition(Vector3f());
    guiCam.SetRotation(Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, -1.0f, 0.0f));
    guiCam.MinOrthoBounds = Vector3f(0.0f, 0.0f, -10.0f);
    guiCam.MaxOrthoBounds = Vector3f(windowSize, 10.0f);
    guiCam.PerspectiveInfo.Width = windowSize.x;
    guiCam.PerspectiveInfo.Height = windowSize.y;

    Matrix4f viewM, projM;
    guiCam.GetViewTransform(viewM);
    guiCam.GetOrthoProjection(projM);
    RenderInfo info(Manager->GetTotalElapsedSeconds(), &guiCam, &viewM, &projM);

    GUIManager.Render(frameSeconds, info);
}
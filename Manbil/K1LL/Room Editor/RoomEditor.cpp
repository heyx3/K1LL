#include "RoomEditor.h"

#include "../../Rendering/GUI/GUI Elements/GUISlider.h"
#include "../../Rendering/GUI/GUI Elements/GUISelectionBox.h"
#include "EditorView.h"


#include <iostream>
namespace
{
    void PauseConsole(void)
    {
        char dummy;
        std::cin >> dummy;
    }

    //The GUI's camera is positioned so the origin is at the bottom-left of the window,
    //    and the width/height of the view is equal to the pixel width/height.
    Camera GetGUICam(Vector2u windowSize)
    {
        Camera cam(Vector3f(), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, -1.0f, 0.0f));
        cam.MinOrthoBounds = Vector3f(0.0f, 0.0f, -10.0f);
        cam.MaxOrthoBounds = Vector3f((float)windowSize.x, (float)windowSize.y, 10.0f);
        cam.PerspectiveInfo.Width = windowSize.x;
        cam.PerspectiveInfo.Height = windowSize.y;
        return cam;
    }
}



RoomEditor::RoomEditor(void)
    : windowSize(800, 600), editorMaterials(0), textRenderer(0),
      SFMLOpenGLWorld(800, 600, sf::ContextSettings())
{
}

sf::VideoMode RoomEditor::GetModeToUse(unsigned int windowW, unsigned int windowH)
{
    //Change this return value to change the window resolution mode.
    //To use native fullscreen, return "sf::VideoMode::getFullscreenModes()[0];".
    return sf::VideoMode(windowW, windowH);
}
std::string RoomEditor::GetWindowTitle(void)
{
    //Change this to change the string on the window's title-bar
    //    (assuming it has a title-bar).
    return "Room Editor";
}
sf::Uint32 RoomEditor::GetSFStyleFlags(void)
{
    //Change this to change the properties of the window.
    return sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close;
}

void RoomEditor::InitializeWorld(void)
{
    SFMLOpenGLWorld::InitializeWorld();
    //If there was an error initializing the game, don’t bother with
    //    the rest of initialization.
    if (IsGameOver())
    {
        return;
    }
    
    DrawingQuad::InitializeQuad();
    TextRenderer::InitializeSystem();

    textRenderer = new TextRenderer();
    editorMaterials = new EditorMaterialSet(*textRenderer);

    std::string err;
    EditorView* view = new EditorView(err);
    if (!Assert(err.empty(), "Error creating editor view", err))
    {
        return;
    }
    manager.RootElement = GUIElementPtr(view);

    manager.RootElement->SetBounds(Box2D(0.0f, (float)windowSize.x,
                                         -(float)windowSize.y, 0.0f));
}
void RoomEditor::OnWorldEnd(void)
{
    manager.RootElement.reset();

    if (editorMaterials != 0)
    {
        delete editorMaterials;
        editorMaterials = 0;
    }
    if (textRenderer != 0)
    {
        delete textRenderer;
        textRenderer = 0;
    }

    TextRenderer::DestroySystem();
    DrawingQuad::DestroyQuad();
}

void RoomEditor::UpdateWorld(float elapsedSeconds)
{
    sf::Vector2i mPos = sf::Mouse::getPosition();
    sf::Vector2i mPosFinal = mPos - GetWindow()->getPosition() - sf::Vector2i(5, 30);
    mPosFinal.y -= windowSize.y;

    manager.Update(elapsedSeconds, Vector2i(mPosFinal.x, mPosFinal.y),
                   sf::Mouse::isButtonPressed(sf::Mouse::Left));
}
void RoomEditor::RenderOpenGL(float elapsedSeconds)
{
    //Set up rendering state.
    //Modify these constructors to change various aspects of how rendering is done.
    ScreenClearer(true, true, false, Vector4f(0.2, 0.2, 0.2f, 0.0f)).ClearScreen();
    RenderingState(RenderingState::C_NONE).EnableState();
    glViewport(0, 0, windowSize.x, windowSize.y);

    if (manager.RootElement.get() != 0)
    {
        //Build the camera info.
        Camera cam = GetGUICam(windowSize);
        Matrix4f viewM, projM;
        cam.GetViewTransform(viewM);
        cam.GetOrthoProjection(projM);

        RenderInfo info(GetTotalElapsedSeconds(), &cam, &viewM, &projM);
        manager.Render(elapsedSeconds, info);
    }
}

void RoomEditor::OnInitializeError(std::string errorMsg)
{
    Assert(false, "Error initializing the game: ", errorMsg);
}
void RoomEditor::OnWindowResized(unsigned int newWidth, unsigned int newHeight)
{
    windowSize.x = newWidth;
    windowSize.y = newHeight;
}

void RoomEditor::OnOtherWindowEvent(sf::Event& windowEvent)
{
    if (windowEvent.type == sf::Event::MouseWheelMoved)
    {
        ((EditorView*)manager.RootElement.get())->MouseWheelDelta += windowEvent.mouseWheel.delta;
    }
}

bool RoomEditor::Assert(bool test, std::string errorIntro, const std::string& error)
{
    if (!test)
    {
        std::cout << errorIntro << ": " << error << "\n";
        PauseConsole();
        EndWorld();
        return false;
    }

    return true;
}
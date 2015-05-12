#include "LevelEditor.h"



LevelEditor::LevelEditor(const std::string& levelFileName, PageManager* manager)
    : Page(manager), worldViewBounds(0.0f, 20.0f, 0.0f, 20.0f)
{

}
LevelEditor::~LevelEditor(void)
{

}


void LevelEditor::Update(Vector2i mousePos, float frameseconds)
{

}
void LevelEditor::Render(float frameSeconds)
{

}


void LevelEditor::OnWindowResized(void)
{

}
void LevelEditor::OnCloseWindow(void)
{

}

void LevelEditor::OnOtherWindowEvent(sf::Event& windowEvent)
{
    if (windowEvent.type == sf::Event::MouseWheelMoved)
    {

    }
}
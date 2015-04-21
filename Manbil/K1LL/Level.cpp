#include "Level.h"


void Level::Update(float elapsed)
{
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        Rooms[i]->Update(elapsed);
    }
}
void Level::Render(float elapsed, const RenderInfo& info)
{

}
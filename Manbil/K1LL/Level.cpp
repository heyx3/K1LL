#include "Level.h"


Level Level::Instance = Level();

void Update(float elapsed)
{
    for (unsigned int i = 0; i < Rooms.size(); ++i)
    {
        Rooms[i]->Update(elapsed);
    }
}
void Render(float elapsed, const RenderInfo& info)
{

}
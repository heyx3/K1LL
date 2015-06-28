#include "PuncherBullet.h"

#include "../../../Content/WeaponConstants.h"


ActorPtr PuncherBulletPool::instance = ActorPtr();


ActorPtr PuncherBulletPool::CreatePool(Level* lvl)
{
    instance = ActorPtr(new PuncherBulletPool(lvl));
    return instance;
}

bool PuncherBullet::Update(Level* level, float elapsedSeconds)
{
    //TODO: Add line-segment-cast support to Level class.
    assert(false);

    return false;
}
void PuncherBullet::Render(Level* level, float elapsedSeconds, const RenderInfo& info)
{
    //TODO: Render the bullet.
    assert(false);
}


#define PUNCH_BUF_SIZE WeaponConstants::Instance.PuncherBufferSize

PuncherBulletPool::PuncherBulletPool(Level* lvl)
    : Actor(lvl)
{
    isAllocated = new bool[PUNCH_BUF_SIZE];
    bullets = new PuncherBullet[PUNCH_BUF_SIZE];
    activeBullets.reserve(16);
    
    for (unsigned int i = 0; i < PUNCH_BUF_SIZE; ++i)
        isAllocated[i] = false;
}
PuncherBulletPool::~PuncherBulletPool(void)
{
    delete[] isAllocated, bullets;
}


PuncherBullet* PuncherBulletPool::NewBullet(Vector3f pos, Vector3f velocity)
{
    return GetInstance()->NewBulletInstance(pos, velocity);
}
PuncherBullet* PuncherBulletPool::NewBulletInstance(Vector3f pos, Vector3f velocity)
{
    //Keep cycling through the bullets until an unused one is found.
    unsigned int startVal = nextBullet;
    while (isAllocated[nextBullet])
    {
        nextBullet = (nextBullet + 1) % PUNCH_BUF_SIZE;

        //If there aren't any bullets left to allocate, crash.
        assert(nextBullet != startVal);
    }

    //Initialize and return the bullet.
    isAllocated[nextBullet] = true;
    activeBullets.push_back(nextBullet);
    PuncherBullet& bullet = bullets[nextBullet];
    bullet = PuncherBullet(pos, velocity);
    return &bullet;
}

bool PuncherBulletPool::Update(float elapsedSeconds)
{
    for (int i = 0; i < (int)activeBullets.size(); ++i)
    {
        unsigned int index = activeBullets[i];

        if (bullets[index].Update(GetLevel(), elapsedSeconds))
        {
            activeBullets.erase(activeBullets.begin() + i);
            isAllocated[index] = false;
            i -= 1;
        }
    }

    return false;
}
void PuncherBulletPool::Render(float elapsedSeconds, const RenderInfo& info)
{
    for (unsigned int i = 0; i < activeBullets.size(); ++i)
    {
        bullets[activeBullets[i]].Render(GetLevel(), elapsedSeconds, info);
    }
}
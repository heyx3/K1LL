#include "PuncherBullet.h"

#include "../../../Content/WeaponConstants.h"
#include "../../../Content/BulletContent.h"
#include "../../Level/Level.h"
#include "../Player.h"


ActorPtr PuncherBulletPool::instance = ActorPtr();


ActorPtr PuncherBulletPool::CreatePool(Level* lvl)
{
    instance = ActorPtr(new PuncherBulletPool(lvl));
    return instance;
}

bool PuncherBullet::Update(Level* level, float elapsedSeconds)
{
    //Step the bullet forward.
    Vector3f oldPos = Pos;
    Pos += (Velocity * elapsedSeconds);

    //See if any players were hit.
    Box2D lineBnds = Box2D(Mathf::Min(oldPos.x, Pos.x), Mathf::Max(oldPos.x, Pos.x),
                           Mathf::Min(oldPos.y, Pos.y), Mathf::Max(oldPos.y, Pos.y));
    for (unsigned int i = 0; i < level->Players.size(); ++i)
    {
        Player& player = *level->Players[i];
        if (player.GetBoundingBox2D().Touches(lineBnds))
        {
            auto hitResult = player.GetCollision3D().RayHitCheck(oldPos, Velocity);
            if (hitResult.DidHitTarget && hitResult.HitT >= 0.0f && hitResult.HitT <= elapsedSeconds)
            {
                //TODO: Hurt player.
                return true;
            }
        }
    }

    //Cast a ray segment from the old position to the new position to see if a wall was hit.
    Vector3f outHit;
    float outT;
    Level::RaycastResults castResult = level->CastWallRay(oldPos, Velocity, outHit, outT, elapsedSeconds);
    if (castResult != Level::RR_NOTHING)
    {
        return true;
    }

    return false;
}
void PuncherBullet::Render(Level* level, float elapsedSeconds, const RenderInfo& info)
{
    BulletContent::Instance.RenderPuncherBullet(Pos, Velocity, info);
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
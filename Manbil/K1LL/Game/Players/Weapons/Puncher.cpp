#include "Puncher.h"

#include "../../../Content/ActorContent.h"
#include "../../../Content/WeaponConstants.h"
#include "../../../Content/WeaponContent.h"
#include "../Projectiles/PuncherBullet.h"
#include "../Player.h"
#include "../../../Content/ParticleContent.h"


void Puncher::StartFire(void)
{
    TryFire();

    Weapon::StartFire();
}

void Puncher::Update(float elapsed)
{
    timeSinceFire += elapsed;

    if (IsFiring())
    {
        TryFire();
    }
}
void Puncher::Render(Vector3f pos, Vector3f dir, const RenderInfo& info) const
{
    WeaponContent::Instance.RenderPuncher(pos, dir, info);
}

void Puncher::TryFire(void)
{
    if (timeSinceFire >= WeaponConstants::Instance.PuncherReloadTime)
    {
        timeSinceFire = 0.0f;

        //Calculate the initial position and velocity for the bullet.
        auto posAndDir = Owner->GetWeaponPosAndDir();
        Vector3f pos = posAndDir.first + (posAndDir.second * WeaponConstants::Instance.WeaponLength);
        Vector3f vel = posAndDir.second * WeaponConstants::Instance.PuncherBulletSpeed;
        PuncherBulletPool::GetInstance()->NewBullet(pos, vel);

        //Calculate the tangent/bitangent for the weapon for a particle burst.
        Vector3f tangent, bitangent;
        if (abs(posAndDir.second.z) > 0.999f)
        {
            tangent = Vector3f(1.0f, 0.0f, 0.0f);
            bitangent = Vector3f(0.0f, 1.0f, 0.0f);
        }
        else
        {
            tangent = posAndDir.second.Cross(Vector3f(0.0f, 0.0f, 1.0f));
            bitangent = tangent.Cross(posAndDir.second);
        }

        ParticleContent::Instance.PuncherFire_Burst(pos, posAndDir.second, tangent, bitangent);
    }
}
#include "Puncher.h"

#include "../../../Content/ActorContent.h"
#include "../../../Content/WeaponConstants.h"
#include "../../../Content/WeaponContent.h"
#include "../Projectiles/PuncherBullet.h"


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
        //TODO: Allocate and fire a puncher bullet.
        assert(false);
    }
}
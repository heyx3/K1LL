#pragma once

#include "Weapon.h"


//A weapon that combines the rapid-fire of a machine gun with the strength of a revolver.
class Puncher : public Weapon
{
public:

    Puncher(Level& level) : Weapon(WT_LIGHT, level) { }

    
    virtual void StartFire(void) override;

    virtual void Update(float elapsed) override;
    virtual void Render(Vector3f pos, Vector3f dir, const RenderInfo& info) const override;


private:

    float timeSinceFire = 9999999.0f;

    void TryFire(void);
};
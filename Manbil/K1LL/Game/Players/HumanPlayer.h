#pragma once

#include "Player.h"


//A human-controlled player.
class HumanPlayer : public Player
{
public:

    float RotSpeedLerp = 0.5f;


    HumanPlayer(Vector2f pos);


    virtual void Update(float elapsedSeconds) override;


private:

    static const Vector2i baseMPos;
};
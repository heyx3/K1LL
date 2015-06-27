#pragma once

#include "Player.h"

#include "../InputHandler.h"


//A human-controlled player.
class HumanPlayer : public Player
{
public:

    InputMethods InputType;

    float RotSpeedLerp = 0.5f;


    HumanPlayer(Level* level, InputMethods input, Vector2f pos, Weapon::Ptr weapons[3]);


    virtual void Update(float elapsedSeconds) override;
};
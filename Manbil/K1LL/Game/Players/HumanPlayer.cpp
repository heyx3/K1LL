#include "HumanPlayer.h"



HumanPlayer::HumanPlayer(Level* level, InputMethods input, Vector2f pos, Weapon::Ptr weapons[3])
    : InputType(input), Player(level, pos, weapons)
{

}

void HumanPlayer::Update(float elapsed)
{
    InputHandler::Inputs& inputs = InputHandler::Values[InputType];

    //First, handle rotation input.
    Vector3f side = LookDir.Cross(Vector3f(0.0f, 0.0f, 1.0f));
    Quaternion qPitch(side, inputs.Pitch),
               qYaw(Vector3f(0.0f, 0.0f, 1.0f), inputs.Yaw);

    //Do yaw rotation.
    qYaw.Rotate(LookDir);

    //Try pitch rotation.
    Vector3f oldLook = LookDir;
    qPitch.Rotate(LookDir);
    if (abs(LookDir.Dot(Vector3f(0.0f, 0.0f, 1.0f))) < LevelConstants::Instance.PlayerLookMinDot)
    {
        LookDir = oldLook;
    }

    LookDir.Normalize();

    //Next, handle movement input.
    Vector2f forward = LookDir.XY().Normalized(),
             newSide = -side.XY().Normalized();
    Acceleration += forward * inputs.ForwardMovement * LevelConstants::Instance.PlayerAccel;
    Acceleration += newSide * inputs.RightwardMovement * LevelConstants::Instance.PlayerAccel;


    //Handle firing input. Pretty simple.
    Fire = inputs.Fire;

    //TODO: Handle pause and weapon-switch inputs.


    Player::Update(elapsed);
}
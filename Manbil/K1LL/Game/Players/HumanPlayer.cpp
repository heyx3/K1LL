#include "HumanPlayer.h"



HumanPlayer::HumanPlayer(Level* level, InputMethods input, Vector2f pos)
    : InputType(input), Player(level, pos)
{

}

void HumanPlayer::Update(float elapsed)
{
    InputHandler::Inputs& inputs = InputHandler::Values[InputType];

    //First, handle rotation input.
    Vector3f side = LookDir.Cross(Vector3f(0.0f, 0.0f, 1.0f));
    Quaternion qPitch(side, inputs.Pitch),
               qYaw(Vector3f(0.0f, 0.0f, 1.0f), inputs.Yaw);

    //Try pitch rotation.
    Vector3f oldLook = LookDir;
    qPitch.Rotate(LookDir);
    if (LookDir.Dot(Vector3f(0.0f, 0.0f, 1.0f)) < Constants::Instance.PlayerLookMinDot)
    {
        LookDir = oldLook;
    }

    //Next, do yaw rotation.
    qYaw.Rotate(LookDir);

    //Second, handle movement input.
    Vector2f forward = LookDir.XY().Normalized(),
             newSide(forward.y, forward.x);
    Acceleration += forward * inputs.ForwardMovement * Constants::Instance.PlayerAccel;
    Acceleration += newSide * inputs.RightwardMovement * Constants::Instance.PlayerAccel;


    //TODO: Handle pause, fire, and weapon-switch inputs.
}
#include "HumanPlayer.h"


const Vector2i HumanPlayer::baseMPos = Vector2i(50, 50);


HumanPlayer::HumanPlayer(PageManager* manager, Vector2f pos)
    : Player(manager, pos)
{

}

void HumanPlayer::Update(float elapsed)
{
    //First, handle rotation input.

	sf::Vector2i mousePos = sf::Mouse::getPosition(*Manager->GetWindow());
    sf::Mouse::setPosition(sf::Vector2i(baseMPos.x, baseMPos.y), *Manager->GetWindow());

	Vector2i delta = Vector2i(mousePos.x, mousePos.y) - baseMPos;
    float rotSpeed = Mathf::Lerp(Constants::Instance.MinMouseRotSpeed,
                                 Constants::Instance.MaxMouseRotSpeed,
                                 RotSpeedLerp);
	float yaw = delta.x * rotSpeed * elapsed,
          pitch = delta.y * rotSpeed * elapsed;

    Vector3f side = LookDir.Cross(Vector3f(0.0f, 0.0f, 1.0f));
    Quaternion qPitch(side, pitch),
               qYaw(Vector3f(0.0f, 0.0f, 1.0f), yaw);

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
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        Acceleration += forward * Constants::Instance.PlayerAccel;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        Acceleration += forward * -Constants::Instance.PlayerAccel;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        Acceleration += newSide * Constants::Instance.PlayerAccel;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        Acceleration += newSide * -Constants::Instance.PlayerAccel;
    }
}
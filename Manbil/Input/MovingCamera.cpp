#include "MovingCamera.h"


bool MovingCamera::Update(float elapsedTime)
{
	#pragma region WASD EQ

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		IncrementPosition(GetForward() * MoveSpeed * elapsedTime);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		IncrementPosition(-GetForward() * MoveSpeed * elapsedTime);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		IncrementPosition(GetSideways() * MoveSpeed * elapsedTime);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		IncrementPosition(-GetSideways() * MoveSpeed * elapsedTime);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		IncrementPosition(GetUpward() * MoveSpeed * elapsedTime);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		IncrementPosition(GetUpward() * -MoveSpeed * elapsedTime);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
	{
		return true;
	}

	#pragma endregion

	#pragma region Mouse

	if (Window == 0)
	{
		return false;
	}

	Window->setMouseCursorVisible(!capMouse);
	if (sf::Keyboard::isKeyPressed(ToggleMouseCapKey))
	{
		if (!pressedSpace)
		{
			Window->setMouseCursorVisible(true);
			pressedSpace = true;
			capMouse = !capMouse;

            sf::Mouse::setPosition(Conv(mouseTarget), *Window);
		}
	}
	else
    {
        pressedSpace = false;
    }

	if (!capMouse)
    {
        return false;
    }

	Vector2i mousePos = Conv(sf::Mouse::getPosition(*Window));
	Vector2i delta = mousePos - mouseTarget;

	Vector2f rotAmount = ToV2f(delta) * RotSpeed * elapsedTime;

	sf::Mouse::setPosition(Conv(mouseTarget), *Window);

	AddPitch(rotAmount.y);
	AddYaw(rotAmount.x);

	#pragma endregion

	return false;
}
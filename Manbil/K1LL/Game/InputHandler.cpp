#include "InputHandler.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>

#include "../../Math/Lower Math/Vectors.h"

#include "../Content/Settings.h"


namespace
{
    Vector2i BASE_MOUSE_POS = Vector2i(100, 100);
    float TRIGGER_FIRE_THRESHOLD = 0.25f;
}


std::unordered_map<InputMethods, InputHandler::Inputs> InputHandler::Values = {
    { IM_KEYBOARD_MOUSE, Inputs() },
    { IM_GAMEPAD1, Inputs() },
    { IM_GAMEPAD2, Inputs() },
    { IM_GAMEPAD3, Inputs() },
    { IM_GAMEPAD4, Inputs() },
};
std::unordered_map<InputMethods, bool> InputHandler::UseLeftHanded = {
    { IM_KEYBOARD_MOUSE, false },
    { IM_GAMEPAD1, false },
    { IM_GAMEPAD2, false },
    { IM_GAMEPAD3, false },
    { IM_GAMEPAD4, false },
};


void InputHandler::Update(sf::Window* window, int scrollWheel)
{
    //Do KB+M first.

    Inputs& kbm = Values[IM_KEYBOARD_MOUSE];

    #define KEY_PRESSED(key) sf::Keyboard::isKeyPressed(sf::Keyboard::key)

    //Keyboard.
    if (UseLeftHanded[IM_KEYBOARD_MOUSE])
    {
        kbm.ForwardMovement = ((KEY_PRESSED(Up) || KEY_PRESSED(Numpad5)) ?
                                   1.0f : 0.0f) +
                              ((KEY_PRESSED(Down) || KEY_PRESSED(Numpad2)) ?
                                  -1.0f : 0.0f);
        kbm.RightwardMovement = ((KEY_PRESSED(Right) || KEY_PRESSED(Numpad3)) ?
                                     1.0f : 0.0f) +
                                    ((KEY_PRESSED(Left) || KEY_PRESSED(Numpad1)) ?
                                     -1.0f : 0.0f);
        kbm.SwitchToLight = KEY_PRESSED(RControl) || KEY_PRESSED(Numpad4);
        kbm.SwitchToHeavy = KEY_PRESSED(RShift) || KEY_PRESSED(Numpad6);
        kbm.SwitchToSpecial = KEY_PRESSED(Numpad0) || KEY_PRESSED(Numpad8);
        kbm.Pause = KEY_PRESSED(Return);
    }
    else
    {
        kbm.ForwardMovement = (KEY_PRESSED(W) ? 1.0f : 0.0f) +
                              (KEY_PRESSED(S) ? -1.0f : 0.0f);
        kbm.RightwardMovement = (KEY_PRESSED(D) ? 1.0f : 0.0f) +
                                (KEY_PRESSED(A) ? -1.0f : 0.0f);
        kbm.SwitchToLight = KEY_PRESSED(Num1);
        kbm.SwitchToHeavy = KEY_PRESSED(Num2);
        kbm.SwitchToSpecial = KEY_PRESSED(Num3);
        kbm.Pause = KEY_PRESSED(Escape);
    }
    kbm.NextWeapon = (scrollWheel > 0) || KEY_PRESSED(E);
    kbm.PrevWeapon = (scrollWheel < 0) || KEY_PRESSED(Q);

    //Mouse.
    kbm.Fire = sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
               sf::Mouse::isButtonPressed(sf::Mouse::Right);
    sf::Vector2i sfMousePos = sf::Mouse::getPosition(*window);
    Vector2i delta = Vector2i(sfMousePos.x, sfMousePos.y) - BASE_MOUSE_POS;
    sf::Mouse::setPosition(sf::Vector2i(BASE_MOUSE_POS.x, BASE_MOUSE_POS.y), *window);
    kbm.Yaw = Settings::Instance.MouseSpeedX;
    kbm.Pitch = Settings::Instance.MouseSpeedY;


    //Next, do the joysticks.

    #define SFJ sf::Joystick
    #define CALC_INPUTS_FOR_JOYSTICK(zeroBased, oneBased) \
            { \
                Inputs& gamepad = Values[IM_GAMEPAD ## oneBased]; \
                if (sf::Joystick::isConnected(zeroBased)) \
                { \
                    gamepad.ForwardMovement = 0.01f * SFJ::getAxisPosition(zeroBased, SFJ::Axis::Y); \
                    gamepad.RightwardMovement = 0.01f * SFJ::getAxisPosition(zeroBased, SFJ::Axis::X); \
                    gamepad.Fire = abs(0.01f * SFJ::getAxisPosition(zeroBased, SFJ::Z)) > \
                                   TRIGGER_FIRE_THRESHOLD; \
                     \
                    gamepad.PrevWeapon = SFJ::isButtonPressed(zeroBased, 5); \
                    gamepad.PrevWeapon = SFJ::isButtonPressed(zeroBased, 6); \
                    gamepad.SwitchToLight = SFJ::isButtonPressed(zeroBased, 0) || \
                                            SFJ::isButtonPressed(zeroBased, 7); \
                    gamepad.SwitchToHeavy = SFJ::isButtonPressed(zeroBased, 1) || \
                                            SFJ::isButtonPressed(zeroBased, 8); \
                    gamepad.SwitchToSpecial = SFJ::isButtonPressed(zeroBased, 3) || \
                                              SFJ::isButtonPressed(zeroBased, 9); \
                     \
                    gamepad.Yaw = Settings::Instance.ThumbstickSpeedX * \
                                  0.01f * SFJ::getAxisPosition(zeroBased, SFJ::Axis::U); \
                    gamepad.Pitch = Settings::Instance.ThumbstickSpeedY * \
                                    0.01f * SFJ::getAxisPosition(zeroBased, SFJ::Axis::V); \
                } \
                else \
                { \
                    gamepad = Inputs(); \
                } \
            }
    
    CALC_INPUTS_FOR_JOYSTICK(0, 1);
    CALC_INPUTS_FOR_JOYSTICK(1, 2);
    CALC_INPUTS_FOR_JOYSTICK(2, 3);
    CALC_INPUTS_FOR_JOYSTICK(3, 4);
}
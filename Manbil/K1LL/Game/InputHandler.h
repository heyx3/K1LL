#pragma once

#include <unordered_map>

#include <SFML/Window/Window.hpp>


enum InputMethods
{
    IM_KEYBOARD_MOUSE,
    IM_GAMEPAD1,
    IM_GAMEPAD2,
    IM_GAMEPAD3,
    IM_GAMEPAD4,
};


//A singleton that handles player input for a match.
class InputHandler
{
public:

    //The different inputs a player can use.
    struct Inputs
    {
        float ForwardMovement = 0.0f,
              RightwardMovement = 0.0f;

        bool Fire = false;
        bool Pause = false;

        bool NextWeapon = false,
             PrevWeapon = false;
        bool SwitchToLight = false,
             SwitchToHeavy = false,
             SwitchToSpecial = false;

        float Pitch = 0.0f,
              Yaw = 0.0f;
    };


    static std::unordered_map<InputMethods, Inputs> Values;
    static std::unordered_map<InputMethods, bool> UseLeftHanded;


    static void Update(sf::Window* window, int scrollWheel);
};
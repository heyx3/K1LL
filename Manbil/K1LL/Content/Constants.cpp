#include "Constants.h"


Constants Constants::Instance = Constants();


void Constants::WriteData(DataWriter* writer) const
{
    writer->WriteFloat(CeilingHeight, "Ceiling height");
    writer->WriteFloat(PlayerCollisionRadius, "Player collision radius");
    writer->WriteFloat(PlayerHeight, "Player height");
    writer->WriteFloat(PlayerAccel, "Player acceleration");
    writer->WriteFloat(PlayerMaxSpeed, "Player max speed");
    writer->WriteFloat(PlayerLookMinDot, "Player look min dot product");
    
    writer->WriteFloat(MinMouseRotSpeed, "Min mouse rot speed");
    writer->WriteFloat(MaxMouseRotSpeed, "Max mouse rot speed");
    writer->WriteFloat(MinJoystickRotSpeed, "Min joystick rot speed");
    writer->WriteFloat(MaxJoystickRotSpeed, "Max joystick rot speed");
}

void Constants::ReadData(DataReader* reader)
{
    reader->ReadFloat(CeilingHeight);
    reader->ReadFloat(PlayerCollisionRadius);
    reader->ReadFloat(PlayerHeight);
    reader->ReadFloat(PlayerAccel);
    reader->ReadFloat(PlayerMaxSpeed);
    reader->ReadFloat(PlayerLookMinDot);
    
    reader->ReadFloat(MinMouseRotSpeed);
    reader->ReadFloat(MaxMouseRotSpeed);
    reader->ReadFloat(MinJoystickRotSpeed);
    reader->ReadFloat(MaxJoystickRotSpeed);
}
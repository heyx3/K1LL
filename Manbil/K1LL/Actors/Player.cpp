#include "Player.h"

#include "../Room.h"
#include "../Content/ActorContent.h"


Player::Player(Vector2f pos, Room* startingRoom)
    : CurrentRoom(startingRoom), LookDir(1.0f, 0.0f, 0.0f)
{

}

void Player::Update(float elapsed)
{
    Velocity += Acceleration * elapsed;

    float speedSqr = Velocity.LengthSquared();
    if (speedSqr > Constants::Instance.PlayerMaxSpeed * Constants::Instance.PlayerMaxSpeed)
    {
        Velocity = (Velocity / sqrtf(speedSqr)) * Constants::Instance.PlayerMaxSpeed;
    }

    Pos += Velocity * elapsed;
}
void Player::Render(float elapsed, const RenderInfo& info)
{
    const TeamData& dat = TeamData::GetData(MyTeam);
    ActorContent::Instance.RenderPlayer(Pos, LookDir, dat.TeamColor, dat.TeamMeshIndex, info);
}
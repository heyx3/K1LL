#pragma once

#include "../../../Game Loop/SFMLWorld.h"

#include "../../../Rendering/GUI/GUI Elements/GUITexture.h"

#include "../Players/Player.h"



//A view for a player.
class PlayerViewport : public GUITexture
{
public:

    Level& Lvl;
    Player* Target;
    SFMLWorld* World;


    PlayerViewport(Level& lvl, Player* target, SFMLWorld* world, std::string& outErrorMsg);

    PlayerViewport(const PlayerViewport& cpy) = delete;
    

    //Scales this element by the given amount.
    //Default behavior: enables "DidBoundsChange" and scales this element.
    virtual void ScaleBy(Vector2f scaleAmount) override;
    //Sets this element's scale to the given amount (may be approximate).
    //Default behavior: enables "DidBoundsChange" and sets this element's scale.
    virtual void SetScale(Vector2f newScale) override;

    virtual void Render(float elapsedTime, const RenderInfo& info) override;


private:

    RenderTarget worldRendTarg;
    MTexture2D worldRendColor, worldRendDepth;
};
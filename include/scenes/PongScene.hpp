#pragma once
#include "engine/IScene.hpp"
#include "game/Game.hpp" // your existing Game

class PongScene : public IScene 
{
public:
    bool init(int fbw, int fbh) override { return game_.init(fbw, fbh); }
    void resize(int fbw, int fbh) override { game_.resize(fbw, fbh); }

    void update(const FrameInput& in, float dt) override 
    {
        // Map engine input to game's input (keeps Game reusable anywhere)
        InputState gi{};
        gi.leftUp = in.leftUp; gi.leftDown = in.leftDown;
        gi.rightUp = in.rightUp; gi.rightDown = in.rightDown;
        game_.update(gi, dt);
    }

    void render(SpriteBatch& batch) const override { game_.render(batch); }

    OrthoCamera2D& camera()       override { return game_.camera(); }
    const OrthoCamera2D& camera() const override { return game_.camera(); }

private:
    Game game_;
};

#pragma once
#include "gfx/SpriteBatch.hpp"
#include "gfx/OrthoCamera2D.hpp"

// Per-frame input we care about (extend later without touching scenes)
struct FrameInput 
{
    //used for pong pad movement
    bool leftUp = false, leftDown = false;
    bool rightUp = false, rightDown = false;
    //mouse in pixels - for menu
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseLeftDown = false, mouseLeftPressed = false, mouseLeftRelease = false;

};

class IScene 
{
public:
    virtual ~IScene() = default;
    virtual bool init(int fbw, int fbh) = 0;
    virtual void resize(int fbw, int fbh) = 0;
    virtual void update(const FrameInput& in, float dt) = 0;
    virtual void render(SpriteBatch& batch) const = 0;

    // Camera access so App callbacks (scroll/pan) can modify it
    virtual OrthoCamera2D& camera() = 0;
    virtual const OrthoCamera2D& camera() const = 0;
};

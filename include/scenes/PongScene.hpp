#pragma once
#include "engine/IScene.hpp"
#include "ui/BitmapFont.hpp"

class PongScene : public IScene 
{
public:
    bool init(int fbw, int fbh);
    void resize(int fbw, int fbh);

    void update(const FrameInput& in, float dt);
    void render(SpriteBatch& batch) const;

    void onGoalLeft();
    void onGoalRight();
    void renderText(SpriteBatch& batch, const BitmapFont& font) const;

    const OrthoCamera2D& camera() const { return camera_; }  // const view
    OrthoCamera2D& camera() { return camera_; }  // non-const for input callbacks

private:
    struct Paddle { glm::vec2 pos{}, size{ 0.6f, 3.0f }; float speed = 20.0f; };
    struct Ball { glm::vec2 pos{}, vel{}; float radius = 0.35f; float speed = 12.0f; };
    void reset(bool serveRight);
    void clampPaddles();
    void collideWithWalls();
    void collideWithPaddles();
    static bool circleAabb(const glm::vec2& c, float r,
        const glm::vec2& bmin, const glm::vec2& bmax,
        glm::vec2& outNormal, float& outPen);

private:
    OrthoCamera2D camera_;
    float courtHalfW_ = 10.0f;
    float courtHalfH_ = 10.0f;
    Paddle L_{}, R_{};
    Ball ball_{};
    int scoreLeft_ = 0;
    int scoreRight_ = 0;
};

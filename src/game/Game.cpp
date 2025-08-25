#include "game/Game.hpp"
#include <algorithm>
#include <cstdio>
#include <glm/gtc/constants.hpp>
#include <glm/geometric.hpp>   // for glm::dot
#include <cmath>               // for std::sqrt

using glm::vec2; using glm::vec4;

static inline float clampf(float v, float a, float b) { return std::max(a, std::min(v, b)); }

bool Game::init(int fbw, int fbh) {
    camera_.setViewport(fbw, fbh);
    camera_.setCenter({ 0.0f, 0.0f });
    camera_.setHeightWorld(20.0f); // world height; width derives from aspect

    const float aspect = (fbw > 0 && fbh > 0) ? (float)fbw / (float)fbh : 16.0f / 9.0f;
    courtHalfH_ = 10.0f;
    courtHalfW_ = courtHalfH_ * aspect;

    const float margin = 1.2f;
    L_.pos = { -courtHalfW_ + margin, 0.0f };
    R_.pos = { courtHalfW_ - margin, 0.0f };

    reset(/*serveRight=*/true);
    return true;
}

void Game::resize(int fbw, int fbh) {
    camera_.setViewport(fbw, fbh);
    // keep world height stable (zoom), recompute width by aspect
    const float aspect = (fbw > 0 && fbh > 0) ? (float)fbw / (float)fbh : 16.0f / 9.0f;
    courtHalfW_ = courtHalfH_ * aspect;

    // keep paddles at margin from walls
    const float margin = 1.2f;
    L_.pos.x = -courtHalfW_ + margin;
    R_.pos.x = courtHalfW_ - margin;

    clampPaddles();
}

void Game::reset(bool serveRight) {
    ball_.pos = { 0.0f, 0.0f };
    ball_.speed = 12.0f;
    float dirX = serveRight ? 1.0f : -1.0f;
    // slight random-ish vertical: zero for now; nice next step is randomness
    ball_.vel = vec2{ dirX, 0.0f } *ball_.speed;
}

void Game::clampPaddles() {
    const float lim = courtHalfH_ - L_.size.y * 0.5f;
    L_.pos.y = clampf(L_.pos.y, -lim, lim);
    R_.pos.y = clampf(R_.pos.y, -lim, lim);
}

void Game::update(const InputState& in, float dt) {
    // paddles
    if (in.leftUp)   L_.pos.y += L_.speed * dt;
    if (in.leftDown) L_.pos.y -= L_.speed * dt;
    if (in.rightUp)  R_.pos.y += R_.speed * dt;
    if (in.rightDown)R_.pos.y -= R_.speed * dt;
    clampPaddles();

    // ball integrate
    ball_.pos += ball_.vel * dt;

    // world bounds + scoring
    collideWithWalls();

    // paddle collisions
    collideWithPaddles();
}

void Game::collideWithWalls() {
    // top/bottom bounce
    if (ball_.pos.y + ball_.radius > courtHalfH_) {
        ball_.pos.y = courtHalfH_ - ball_.radius;
        ball_.vel.y = -ball_.vel.y;
    }
    if (ball_.pos.y - ball_.radius < -courtHalfH_) {
        ball_.pos.y = -courtHalfH_ + ball_.radius;
        ball_.vel.y = -ball_.vel.y;
    }
    // left/right scoring
    if (ball_.pos.x - ball_.radius < -courtHalfW_) {
        ++scoreR_; std::printf("Score: L %d  |  R %d\n", scoreL_, scoreR_);
        reset(/*serveRight=*/true);
    }
    if (ball_.pos.x + ball_.radius > courtHalfW_) {
        ++scoreL_; std::printf("Score: L %d  |  R %d\n", scoreL_, scoreR_);
        reset(/*serveRight=*/false);
    }
}

bool Game::circleAabb(const vec2& c, float r, const vec2& bmin, const vec2& bmax,
    vec2& outN, float& outPen)
{
    // closest point on AABB to circle center
    float cx = clampf(c.x, bmin.x, bmax.x);
    float cy = clampf(c.y, bmin.y, bmax.y);
    vec2  d = c - vec2{ cx, cy };
    float d2 = glm::dot(d, d);
    if (d2 > r * r) return false;

    float dist = std::sqrt(std::max(d2, 1e-8f));
    outN = (dist > 0.0f) ? (d / dist) : vec2{ (c.x > (bmin.x + bmax.x) * 0.5f) ? 1.0f : -1.0f, 0.0f };
    outPen = r - dist;
    return true;
}

void Game::collideWithPaddles() {
    const float halfW = L_.size.x * 0.5f;
    const float halfH = L_.size.y * 0.5f;

    // Left paddle
    {
        vec2 bmin = L_.pos - vec2{ halfW, halfH };
        vec2 bmax = L_.pos + vec2{ halfW, halfH };
        vec2 n; float pen;
        if (circleAabb(ball_.pos, ball_.radius, bmin, bmax, n, pen) && ball_.vel.x < 0.0f) {
            // resolve & reflect
            ball_.pos += n * pen;
            // add angle based on hit offset on paddle
            float hit = (ball_.pos.y - L_.pos.y) / halfH; // [-1,1]
            hit = clampf(hit, -1.0f, 1.0f);
            vec2 dir = glm::normalize(vec2{ +1.0f, hit });
            ball_.speed = std::min(ball_.speed * 1.03f, 40.0f);
            ball_.vel = dir * ball_.speed;
        }
    }
    // Right paddle
    {
        vec2 bmin = R_.pos - vec2{ halfW, halfH };
        vec2 bmax = R_.pos + vec2{ halfW, halfH };
        vec2 n; float pen;
        if (circleAabb(ball_.pos, ball_.radius, bmin, bmax, n, pen) && ball_.vel.x > 0.0f) {
            ball_.pos += n * pen;
            float hit = (ball_.pos.y - R_.pos.y) / halfH;
            hit = clampf(hit, -1.0f, 1.0f);
            vec2 dir = glm::normalize(vec2{ -1.0f, hit });
            ball_.speed = std::min(ball_.speed * 1.03f, 40.0f);
            ball_.vel = dir * ball_.speed;
        }
    }
}

void Game::render(SpriteBatch& batch) const {
    auto pushCentered = [&](const glm::vec2& c, const glm::vec2& sz, const glm::vec4& col) {
        Sprite s{};
        s.pos = c - 0.5f * sz;   // center -> bottom-left for SpriteBatch
        s.size = sz;
        s.uv = { 0,0,1,1 };
        s.color = col;
        batch.push(s);
        };

    // Center line (full court height)
    pushCentered({ 0.0f, 0.0f }, { 0.12f, courtHalfH_ * 2.0f }, { 0.5f, 0.5f, 0.5f, 1.0f });

    // Paddles
    pushCentered(L_.pos, L_.size, { 0.9f, 0.9f, 0.9f, 1.0f });
    pushCentered(R_.pos, R_.size, { 0.9f, 0.9f, 0.9f, 1.0f });

    // Ball (square; disc mask later if you want)
    const glm::vec2 ballSz{ ball_.radius * 2.0f, ball_.radius * 2.0f };
    pushCentered(ball_.pos, ballSz, { 1.0f, 1.0f, 1.0f, 1.0f });
}


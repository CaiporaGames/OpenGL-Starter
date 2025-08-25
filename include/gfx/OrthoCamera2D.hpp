#pragma once
#include <algorithm>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class OrthoCamera2D 
{
public:
    void setViewport(int fbw, int fbh) 
    { 
        fbw_ = std::max(1, fbw); 
        fbh_ = std::max(1, fbh); 
    }

    void setCenter(const glm::vec2& c) 
    { 
        center_ = c; 
    }

    glm::vec2 center() const 
    { 
        return center_; 
    }

    void setHeightWorld(float h) 
    { 
        height_ = std::clamp(h, minH_, maxH_); 
    }

    float heightWorld() const 
    { 
        return height_; 
    }

    // Scroll wheel: +y = zoom in, -y = zoom out (sens > 1 is multiplicative step)
    void zoomBy(float wheelY, float sens = 1.2f) 
    {
        if (wheelY == 0.0f) return;

        float factor = (wheelY > 0.0f) ? (1.0f / sens) : sens;
        height_ = std::clamp(height_ * factor, minH_, maxH_);
    }

    // Pan by mouse delta in *screen pixels* (GLFW coords: origin top-left)
    void panPixels(float dxPixels, float dyPixels) 
    {
        auto [l, r, b, t] = lrbt();
        const float wW = r - l, hW = t - b;
        glm::vec2 dWorld{
            dxPixels * (wW / fbw_),        // right drag -> move world right
           -dyPixels * (hW / fbh_)         // down drag  -> move world down
        };
        // Move camera opposite so content follows the cursor (dragging the canvas)
        center_ -= dWorld;
    }

    // Convert screen pixel (sx, sy) to world (z=0), GLFW y is top->down
    glm::vec2 screenToWorld(double sx, double sy) const 
    {
        auto [l, r, b, t] = lrbt();
        float nx = static_cast<float>(sx) / static_cast<float>(fbw_);
        float ny = 1.0f - static_cast<float>(sy) / static_cast<float>(fbh_);
        return { l + nx * (r - l), b + ny * (t - b) };
    }

    // View-projection (world -> NDC)
    glm::mat4 vp() const 
    {
        auto [l, r, b, t] = lrbt();
        return glm::ortho(l, r, b, t, -1.0f, 1.0f);
    }

private:
    struct LRBT { float l, r, b, t; };
    LRBT lrbt() const 
    {
        float aspect = static_cast<float>(fbw_) / static_cast<float>(fbh_);
        float halfH = height_ * 0.5f;
        float halfW = halfH * aspect;
        return { center_.x - halfW, center_.x + halfW, center_.y - halfH, center_.y + halfH };
    }

    glm::vec2 center_{ 0.0f, 0.0f };

    float height_ = 10.0f;   // visible world height (tweak to taste)
    int fbw_ = 1, fbh_ = 1;
    static constexpr float minH_ = 0.25f, maxH_ = 1000.0f;
};

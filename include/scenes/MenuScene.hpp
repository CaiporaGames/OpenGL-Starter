#pragma once
#include "engine/IScene.hpp"
#include "ui/BitmapFont.hpp"

class MenuScene : public IScene 
{
public:
    bool init(int fbw, int fbh) override 
    {
        cam_.setViewport(fbw, fbh);
        cam_.setCenter({ 0.0f, 0.0f });
        cam_.setHeightWorld(20.0f);
        return true;
    }

    void resize(int fbw, int fbh) override { cam_.setViewport(fbw, fbh); }

    void update(const FrameInput& in, float /*dt*/) override 
    {
        // hover & clicks in world units
        glm::vec2 m = cam_.screenToWorld(in.mouseX, in.mouseY);
        hoveredStart_ = pointInRect(m, startCenter_, startSize_);
        hoveredQuit_ = pointInRect(m, quitCenter_, quitSize_);
        if (hoveredStart_ && in.mouseLeftPressed) startRequested_ = true;
        if (hoveredQuit_ && in.mouseLeftPressed) quitRequested_ = true;
    }
    void render(SpriteBatch& batch) const override 
    {
        auto pushRectCentered = [&](glm::vec2 c, glm::vec2 sz, glm::vec4 col) 
        {
            Sprite s{}; 
            s.pos = c - 0.5f * sz; 
            s.size = sz; 
            s.uv = { 0,0,1,1 }; 
            s.color = col; 
            batch.push(s);
        };
        // backdrop panel
        pushRectCentered({ 0,0 }, { 18.0f, 12.0f }, { 0.10f,0.10f,0.12f,1.0f });
        // title bar
        pushRectCentered(titleCenter_, titleSize_, { 0.18f,0.18f,0.22f,1.0f });
        // Start button
        glm::vec4 startCol = hoveredStart_ ? glm::vec4(0.30f, 0.80f, 0.40f, 1.0f) : glm::vec4(0.22f, 0.65f, 0.32f, 1.0f);
        pushRectCentered(startCenter_, startSize_, startCol);
        // Quit button
        glm::vec4 quitCol = hoveredQuit_ ? glm::vec4(0.85f, 0.35f, 0.35f, 1.0f) : glm::vec4(0.70f, 0.25f, 0.25f, 1.0f);
        pushRectCentered(quitCenter_, quitSize_, quitCol);
        // (Optional) draw a thin separator
        pushRectCentered({ 0,-0.5f }, { 12.0f, 0.08f }, { 0.5f,0.5f,0.5f,0.5f });
        // Note: no text yet; add bitmap font later if you want labels
    }

    OrthoCamera2D& camera() override { return cam_; }
    const OrthoCamera2D& camera() const override { return cam_; }

    // App queries these to route transitions
    bool wantsStart() const { return startRequested_; }
    bool wantsQuit()  const { return quitRequested_; }
    void consumeRequests() { startRequested_ = quitRequested_ = false; }

    void renderText(SpriteBatch& batch, const BitmapFont& font) const
    {
        //choose glyph size in world units
        glm::vec2 glyphWH(0.6f, 1.0f);
        float letterSpace = 0.1f;

        auto drawCentered = [&](const char* text, glm::vec2 center, glm::vec2 boxWH, glm::vec4 color)
        {
            std::string s(text);
            glm::vec2 size = font.measure(s, glyphWH, letterSpace, 0.0f);
            //center inside bottom rect
            glm::vec2 bl = center - 0.5f * size + glm::vec2(0.0f, 0.5f * (glyphWH.y - (size.y / 1)));
            font.drawTextBL(batch, s, bl, glyphWH, color, letterSpace, 0.0f);
        };
        glm::vec4 textColor = { 0.05f, 0.05f, 0.05f, 1.0f };
        glm::vec4 titleColor = { 1.0f, 0.0f, 0.0f, 1.0f };
        drawCentered("Awesome Pong", titleCenter_ + glm::vec2(-4.0f, -1.0f), titleSize_, titleColor);
        drawCentered("START", startCenter_ + glm::vec2(-1.5f, -1.0f), startSize_, textColor);
        drawCentered("QUIT", quitCenter_ + glm::vec2(-1.5f, -1.0f), startSize_, textColor);
    }

private:
    static bool pointInRect(const glm::vec2& p, const glm::vec2& c, const glm::vec2& sz) {
        glm::vec2 h = 0.5f * sz;
        return p.x >= c.x - h.x && p.x <= c.x + h.x && p.y >= c.y - h.y && p.y <= c.y + h.y;
    }

    OrthoCamera2D cam_;

    // layout in world units (works for any aspect due to ortho height)
    glm::vec2 titleCenter_{ 0,4.5f }, titleSize_{ 16.0f, 2.0f };
    glm::vec2 startCenter_{ 0.0f, 1.0f }, startSize_{ 8.0f, 2.0f };
    glm::vec2 quitCenter_{ 0.0f, -2.0f }, quitSize_{ 8.0f, 2.0f };

    bool hoveredStart_ = false, hoveredQuit_ = false;
    bool startRequested_ = false, quitRequested_ = false;

};

#pragma once
#include "gfx/SpriteBatch.hpp"
#include "gfx/OrthoCamera2D.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct App; // fwd

// Per-frame input (extendable without touching scenes)
struct FrameInput
{
    // --- Existing (pong/menu) ---
    bool leftUp = false, leftDown = false;
    bool rightUp = false, rightDown = false;
    double mouseX = 0.0, mouseY = 0.0;
    bool mouseLeftDown = false, mouseLeftPressed = false, mouseLeftReleased = false;

    // --- New (3D/editor-friendly), optional to use ---
    // raw deltas in pixels this frame
    double mouseDeltaX = 0.0, mouseDeltaY = 0.0;
    // wheel >0 up, <0 down
    double wheelY = 0.0;
    // useful buttons/mods
    bool rmbDown = false, mmbDown = false, lshiftDown = false, lctrlDown = false;
};

class IScene
{
public:
    virtual ~IScene() = default;

    // Lifecycle
    // fbw/fbh are FRAMEBUFFER size (accounting for DPI)
    virtual bool init(int fbw, int fbh) = 0;

    // Strongly preferred: single resize hook using framebuffer size.
    virtual void onFramebufferResize(int fbw, int fbh) = 0;

    // Legacy resize kept for compatibility; default forwards to onFramebufferResize().
    // You can remove this after migrating old scenes.
    virtual void resize(int fbw, int fbh) { onFramebufferResize(fbw, fbh); }

    // Frame
    virtual void update(const FrameInput& in, float dt) = 0;

    // Rendering split: 3D first, then 2D (SpriteBatch), then ImGui
    virtual void render3D(App* app) = 0;

    // 2D render pass; SpriteBatch provided by App to keep batching centralized.
    virtual void render2D(App* app, SpriteBatch& batch) const = 0;

    // Editor/overlay
    virtual void onImGui(App* app) = 0;

    // --- Cameras ---

    // 3D camera (ECS CameraComponent). Used by your 3D shaders/passes.
    virtual glm::mat4 activeCameraVP() const = 0;
    virtual glm::vec3 activeCameraPos() const = 0;

    // 2D UI camera kept explicitly for SpriteBatch.
    // This replaces the old "scene-wide camera()" which forced Ortho for everything.
    virtual OrthoCamera2D& uiCamera() = 0;
    virtual const OrthoCamera2D& uiCamera() const = 0;

    // --- Legacy compatibility (optional) ---
    // Old code calling: render(SpriteBatch&) — provide a no-op default so existing
    // scenes compile while you migrate to render2D(App*, SpriteBatch&).
    virtual void render(SpriteBatch& /*batch*/) const {}
};

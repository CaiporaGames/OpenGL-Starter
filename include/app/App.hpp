#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "gfx/TriangleRenderer.hpp"
#include "gfx/SpriteBatch.hpp"
#include "gfx/Texture2D.hpp"
#include "ui/BitmapFont.hpp"
#include "gfx/OrthoCamera2D.hpp"
#include "engine/IScene.hpp"
#include "scenes/PongScene.hpp"
#include "scenes/MenuScene.hpp"
#include "core/camera/OrbitCamera3D.hpp"
#include "render/Mesh3D.hpp"
#include "render/DebugLines3D.hpp"
#include "render/DebugCross3D.hpp"
#include "core/raycast.hpp"
#include "core/camera/ScreenRay.hpp"
#include "render/BoxWire3D.hpp"
#include "core/aabb.hpp"
#include "core/frustum.hpp"

class App 
{
public:
    App() = default;
    ~App();

    bool init(int w = 800, int h = 600, const char* title = "GL App");
    void run();

    static void onError(int code, const char* desc);
    static void onKey(GLFWwindow* win, int key, int sc, int action, int mods);
    static void onScroll(GLFWwindow* win, double xoff, double yoff);
    static void onMouseButton(GLFWwindow* win, int button, int action, int mods);
    static void onCursorPos(GLFWwindow* win, double x, double y);

private:
    GLFWwindow* window_ = nullptr;
    SpriteBatch spriteBatch_;
    std::unique_ptr<IScene> scene_;   // <— host ANY scene

    Texture2D fontTex_;
    BitmapFont uiFont_;
    GLuint whiteTex_ = 0;
    // fixed-step accumulator
    double acc_ = 0.0;
    int fbw_ = 0, fbh_ = 0;
    bool prevMouseLeftDown_ = false;

    // pan state for MMB drag
    bool   panning_ = false;
    double lastX_ = 0.0, lastY_ = 0.0;

    // 3D camera
    ShaderProgram basic3D_;
    GLint uMVP_ = -1;
    GLint uColor_ = -1;
    Mesh3D cube_;
    core::OrbitCamera3D cam3D_;
    bool orbiting3D_ = false;

    //model cube
    glm::mat4 cubeModel_{ 1.0f };
    DebugLines3D triLines_;
    DebugCross3D hitCross_;
    BoxWire3D box_;
    core::AABB cubeAABBModel_{};
    core::AABB cubeAABBWorld_{};
    bool pickHasHit_ = false;

    //stats 
    float fps_ = 0.0f;
    double fpsAccum_ = 0.0f;
    int fpsFrames_ = 0;

    //frustum
    bool cubeVisible_ = true;
    int objTotal_ = 1;
    int objVisible_ = 1;
};

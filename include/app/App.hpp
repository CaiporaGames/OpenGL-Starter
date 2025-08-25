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

class App {
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
};

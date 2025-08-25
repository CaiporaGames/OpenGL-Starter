#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>
#include <glm/vec4.hpp>
#include <chrono>
#include <algorithm> 

// If you kept stbi_set_flip_vertically_on_load(true), row 0 = bottom row.
// cols, rows = grid size. frame = 0..(cols*rows-1)
static inline glm::vec4 gridUV(int frame, int cols, int rows) {
    const int cx = frame % cols;
    const int cy = frame / cols;           // bottom-to-top order
    const float du = 1.0f / cols;
    const float dv = 1.0f / rows;
    const float u0 = cx * du;
    const float v0 = cy * dv;
    return { u0, v0, u0 + du, v0 + dv };   // (u0, v0, u1, v1)
}

void App::onError(int code, const char* desc) {
    std::fprintf(stderr, "[GLFW %d] %s\n", code, desc);
}
void App::onKey(GLFWwindow* win, int key, int, int action, int) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(win, GLFW_TRUE);
}

void App::onScroll(GLFWwindow* win, double, double yoff) {
    if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win))) {
        if (app->scene_) app->scene_->camera().zoomBy(static_cast<float>(yoff), 1.2f);
    }
}

void App::onMouseButton(GLFWwindow* win, int button, int action, int) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app) return;
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) { app->panning_ = true; glfwGetCursorPos(win, &app->lastX_, &app->lastY_); }
        else if (action == GLFW_RELEASE) { app->panning_ = false; }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double sx, sy; glfwGetCursorPos(win, &sx, &sy);
        auto world = app->scene_->camera().screenToWorld(sx, sy);
        std::printf("Pick world: (%.3f, %.3f)\n", world.x, world.y);
    }
}

void App::onCursorPos(GLFWwindow* win, double x, double y) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app || !app->panning_) return;
    double dx = x - app->lastX_, dy = y - app->lastY_;
    if (app->scene_) app->scene_->camera().panPixels(static_cast<float>(dx), static_cast<float>(dy));
    app->lastX_ = x; app->lastY_ = y;
}



bool App::init(int w, int h, const char* title) 
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    glfwSetErrorCallback(App::onError);
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    #endif

    window_ = glfwCreateWindow(w, h, title, nullptr, nullptr);

    if (!window_) { glfwTerminate(); return false; }

    glfwMakeContextCurrent(window_);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        std::fprintf(stderr, "GLAD load failed\n");
        return false;
    }
    glfwSwapInterval(1);
    glfwSetKeyCallback(window_, App::onKey);

    // (optional but recommended for PNG with transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetWindowUserPointer(window_, this);                 // allow callbacks to reach this App
    glfwSetScrollCallback(window_, App::onScroll);
    glfwSetMouseButtonCallback(window_, App::onMouseButton);
    glfwSetCursorPosCallback(window_, App::onCursorPos);


    // init renderer with batch shaders + a texture (all sprites use this for now)
    if (!spriteBatch_.init("shaders/sprite_batch.vert",
        "shaders/sprite_batch.frag",
        "assets/white.png", /*maxSprites*/ 2000))
        return false;


    whiteTex_ = spriteBatch_.texture();   // cache the white texture id

    //load font texture, nearest for crisp pixels
    if (!fontTex_.load("assets/Panda.png", true)) return false;

    uiFont_.text = fontTex_.id;
    uiFont_.textureWidth = fontTex_.width;
    uiFont_.textureHeight = fontTex_.height;
    uiFont_.columns = 32;
    uiFont_.rows = 3;
    uiFont_.cellPixelWidth = 8;
    uiFont_.cellPixelHeight = 8;
    uiFont_.first = 32;
    uiFont_.last = 127;
    // Initial framebuffer size
    glfwGetFramebufferSize(window_, &fbw_, &fbh_);
    glViewport(0, 0, fbw_, fbh_);

    // Create current scene
    scene_ = std::make_unique<MenuScene>();
    if (!scene_->init(fbw_, fbh_)) return false;

    acc_ = 0.0;
    return true;
}

void App::run() 
{
    using clock = std::chrono::steady_clock;   // monotonic
    auto prev = clock::now();

    const double dtFixed = 1.0 / 120.0;

    while (!glfwWindowShouldClose(window_)) 
    {
        glfwPollEvents();

        // Resize
        int w, h;
        glfwGetFramebufferSize(window_, &w, &h);
        if (w != fbw_ || h != fbh_) 
        {
            fbw_ = w; fbh_ = h;
            glViewport(0, 0, fbw_, fbh_);
            if (scene_) scene_->resize(fbw_, fbh_);
        }

        // Input keyboard
        FrameInput in{};
        in.leftUp = glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS;
        in.leftDown = glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS;
        in.rightUp = glfwGetKey(window_, GLFW_KEY_UP) == GLFW_PRESS;
        in.rightDown = glfwGetKey(window_, GLFW_KEY_DOWN) == GLFW_PRESS;

        //Input mouse
        double mx, my;
        glfwGetCursorPos(window_, &mx, &my);
        const bool leftDown = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        in.mouseX = mx;
        in.mouseY = my;
        in.mouseLeftDown = leftDown;
        in.mouseLeftPressed = leftDown && !prevMouseLeftDown_;
        in.mouseLeftRelease = !leftDown && prevMouseLeftDown_;
        prevMouseLeftDown_ = leftDown;

        // --- Timing / updates ---
        auto now = clock::now();
        double frameDt = std::chrono::duration<double>(now - prev).count();
        prev = now;

        frameDt = std::min(frameDt, 0.25);  // clamp big spikes
        acc_ += frameDt;

        int steps = 0, kMaxSteps = 8;       // avoid spiral-of-death
        while (acc_ >= dtFixed && steps < kMaxSteps) {
            if (scene_) scene_->update(in, static_cast<float>(dtFixed));
            acc_ -= dtFixed;
            ++steps;
        }
        // If no fixed step ran, do a small variable-step so things still move
        if (steps == 0) {
            double dtVar = std::min(acc_, 0.033); // ~33 ms cap
            if (scene_) scene_->update(in, static_cast<float>(dtVar));
            acc_ = 0.0;
        }

        if (auto* menu = dynamic_cast<MenuScene*>(scene_.get()))
        {
            if (menu->wantsQuit())
            {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
                menu->consumeRequests();
            }
            else if (menu->wantsStart())
            {
                menu->consumeRequests();
                scene_ = std::make_unique<PongScene>();
                scene_->init(fbw_, fbh_);
                continue;
            }
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        if (scene_) 
        {
            //UI 
            spriteBatch_.beginWithVP(scene_->camera().vp());
            spriteBatch_.setTexture(whiteTex_);
            spriteBatch_.setSampleMode(0);            // normal RGBA
            scene_->render(spriteBatch_);
            spriteBatch_.endAndDraw();

            //TEXT
            spriteBatch_.beginWithVP(scene_->camera().vp());
            spriteBatch_.setTexture(uiFont_.text);
            spriteBatch_.setSampleMode(2);            // red-as-alpha (inverted)
            //Only menu render text
            if (auto* menu = dynamic_cast<MenuScene*>(scene_.get()))
            {
                menu->renderText(spriteBatch_, uiFont_);
            }
            spriteBatch_.endAndDraw();
        }
        glfwSwapBuffers(window_);
    }
}

App::~App() {
    spriteBatch_.shutdown();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

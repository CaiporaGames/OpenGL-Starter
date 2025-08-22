#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>
#include <glm/vec4.hpp>

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
        app->camera_.zoomBy(static_cast<float>(yoff), 1.2f); // 20% per notch
    }
}

void App::onMouseButton(GLFWwindow* win, int button, int action, int) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app) return;

    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            app->panning_ = true;
            glfwGetCursorPos(win, &app->lastX_, &app->lastY_);
        }
        else if (action == GLFW_RELEASE) {
            app->panning_ = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Example: print world coordinate at mouse for picking
        double sx, sy; glfwGetCursorPos(win, &sx, &sy);
        glm::vec2 world = app->camera_.screenToWorld(sx, sy);
        std::printf("Pick world: (%.3f, %.3f)\n", world.x, world.y);
    }
}

void App::onCursorPos(GLFWwindow* win, double x, double y) {
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app || !app->panning_) return;

    const double dx = x - app->lastX_;
    const double dy = y - app->lastY_;
    app->camera_.panPixels(static_cast<float>(dx), static_cast<float>(dy));
    app->lastX_ = x; app->lastY_ = y;
}


bool App::init(int w, int h, const char* title) {
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

    // init camera viewport once
    int fbw = 0, fbh = 0; glfwGetFramebufferSize(window_, &fbw, &fbh);
    camera_.setViewport(fbw, fbh);
    camera_.setCenter({ 0.0f, 0.0f });
    camera_.setHeightWorld(10.0f); // shows 10 world units vertically at start


    // init renderer with batch shaders + a texture (all sprites use this for now)
    if (!renderer_.init("shaders/sprite_batch.vert",
        "shaders/sprite_batch.frag",
        "assets/run.png", /*maxSprites*/ 2000))
        return false;

    // Center window (optional)
    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    int ax, ay, aw, ah; glfwGetMonitorWorkarea(mon, &ax, &ay, &aw, &ah);
    int ww, wh; glfwGetWindowSize(window_, &ww, &wh);
    glfwSetWindowPos(window_, ax + (aw - ww) / 2, ay + (ah - wh) / 2);

    std::printf("Renderer  : %s\n", (const char*)glGetString(GL_RENDERER));
    std::printf("GL Version: %s\n", (const char*)glGetString(GL_VERSION));
    return true;
}

void App::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        int fbw = 0, fbh = 0; glfwGetFramebufferSize(window_, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);
        glClearColor(0.07f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --- begin a batch frame ---
        camera_.setViewport(fbw, fbh);                 // keep camera aware of current size
        renderer_.beginWithVP(camera_.vp());           // supply camera VP to shader

        // --- Animation state ---
        static double prev = glfwGetTime();
        double now = glfwGetTime();
        float dt = float(now - prev);
        prev = now;

        static int frame = 0;
        static float accumulator = 0.0f;

        const int   COLS = 8;          // grid: 4 x 2
        const int   ROWS = 1;
        const int   TOTAL_FRAMES = COLS * ROWS;
        const float FPS = 12.0f;       // 8–16 FPS looks nice for run cycles
        const float FRAME_TIME = 1.0f / FPS;

        accumulator += dt;
        while (accumulator >= FRAME_TIME) {
            frame = (frame + 1) % TOTAL_FRAMES;
            accumulator -= FRAME_TIME;
        }

        // --- Push the animated sprite ---
        Sprite runner{};
        runner.pos = { -0.75f, -0.75f }; // bottom-left so that it’s centered at (0,0)
        runner.size = { 1.5f, 1.5f };     // ~15% of screen height at heightWorld=10
        runner.uv = gridUV(frame, COLS, ROWS);
        runner.color = { 1, 1, 1, 1 };         // tint/alpha
        renderer_.push(runner);

        // (push other sprites as you like...)
        renderer_.endAndDraw();


        glfwSwapBuffers(window_);
    }
}

App::~App() {
    renderer_.shutdown();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

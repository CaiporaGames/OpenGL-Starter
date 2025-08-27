#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>
#include <glm/vec4.hpp>
#include <chrono>
#include <algorithm> 
#include "gfx/GLDebug.hpp"

// If you kept stbi_set_flip_vertically_on_load(true), row 0 = bottom row.
// cols, rows = grid size. frame = 0..(cols*rows-1)
static inline glm::vec4 gridUV(int frame, int cols, int rows) 
{
    const int cx = frame % cols;
    const int cy = frame / cols;           // bottom-to-top order
    const float du = 1.0f / cols;
    const float dv = 1.0f / rows;
    const float u0 = cx * du;
    const float v0 = cy * dv;
    return { u0, v0, u0 + du, v0 + dv };   // (u0, v0, u1, v1)
}

void App::onError(int code, const char* desc) 
{
    std::fprintf(stderr, "[GLFW %d] %s\n", code, desc);
}

void App::onKey(GLFWwindow* win, int key, int, int action, int) 
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(win, GLFW_TRUE);
}

void App::onScroll(GLFWwindow* win, double, double yoff) 
{
    if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win))) 
    {
        //SHIFT + wheel -> 3D cam dolly; else -> 2D zoom
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        {
            app->cam3D_.dolly((float)yoff, 1.2f);
        }
        else if (app->scene_) app->scene_->camera().zoomBy(static_cast<float>(yoff), 1.2f);
    }
}

void App::onMouseButton(GLFWwindow* win, int button, int action, int) 
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            app->orbiting3D_ = true;
            glfwGetCursorPos(win, &app->lastX_, &app->lastY_);
        }
        else if (action == GLFW_RELEASE)
        {
            app->orbiting3D_ = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) 
    {
        if (action == GLFW_PRESS) 
        { 
            app->panning_ = true; 
            glfwGetCursorPos(win, &app->lastX_, &app->lastY_); 
        }
        else if (action == GLFW_RELEASE) 
        { 
            app->panning_ = false; 
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
    {
        double sx, sy; glfwGetCursorPos(win, &sx, &sy);
        auto world = app->scene_->camera().screenToWorld(sx, sy);
        std::printf("Pick world: (%.3f, %.3f)\n", world.x, world.y);
    }
}

void App::onCursorPos(GLFWwindow* win, double x, double y) 
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app) return;

    double dx = x - app->lastX_, dy = y - app->lastY_;

    if (app->orbiting3D_) 
    {
        // Upwards drag should pitch up (negative dy)
        app->cam3D_.tumble((float)dx, (float)-dy, 0.2f);
    }
    else if (app->panning_) 
    {
        if (app->scene_) app->scene_->camera().panPixels((float)dx, (float)dy);
    }

    app->lastX_ = x; app->lastY_ = y;
}


bool App::init(int w, int h, const char* title) 
{
    //Realtime logging → When you print debug info (FPS, input events, OpenGL errors, etc.), you want to see it instantly
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
    //Enabling the debbuging
    GLDebug::enable();

    glfwSwapInterval(1);
    glfwSetKeyCallback(window_, App::onKey);

    // (Recommended for PNG with transparency)
    //Blend the new pixel’s color with what’s already in the framebuffer.
    glEnable(GL_BLEND);
    //We apply transparece in the finalColor = srcColor * alpha + dstColor * (1 - alpha)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Allow callbacks to reach this App. GLFW doesn’t have built-in C++ object support, so you must store this.
    glfwSetWindowUserPointer(window_, this);                 
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

    //3D camera setup
    cam3D_.setTarget({0,0,0});
    cam3D_.setDistance(6.0f);
    cam3D_.setLens(60.0f, 0.1f, 100.0f);

    //load 3D shader + uniforms
    if (!basic3D_.loadFromFiles("shaders/basic3d.vert", "shaders/basic3d.frag")) return false;
    uMVP_ = basic3D_.uniformLocation("u_MVP");
    uColor_ = basic3D_.uniformLocation("u_Color");

    if (!cube_.initColoredCube()) return false;

    // Initial framebuffer size
    glfwGetFramebufferSize(window_, &fbw_, &fbh_);
    glViewport(0, 0, fbw_, fbh_);
    cam3D_.setViewport(fbw_, fbh_);
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
        cam3D_.setViewport(fbw_, fbh_);
        if (w != fbw_ || h != fbh_) 
        {
            fbw_ = w; fbh_ = h;
            glViewport(0, 0, fbw_, fbh_);
            cam3D_.setViewport(fbw_, fbh_);
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
        in.mouseLeftReleased = !leftDown && prevMouseLeftDown_;
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

        //Menu scene
        if (auto* scene = dynamic_cast<MenuScene*>(scene_.get()))
        {
            if (scene->wantsQuit())
            {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
                scene->consumeRequests();
            }
            else if (scene->wantsStart())
            {
                scene->consumeRequests();
                scene_ = std::make_unique<PongScene>();
                scene_->init(fbw_, fbh_);
                continue;
            }
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //For 3D mesh
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        //3D pass, one draw call
        {
            basic3D_.use();
            //model: scale the unit to something noticeable
            glm::mat4 M(1.0f);
            M = glm::scale(M, glm::vec3(3.0f));
            const glm::mat4 MVP = cam3D_.vp() * M;
            if (uMVP_ != -1) glUniformMatrix4fv(uMVP_, 1, GL_FALSE, &MVP[0][0]);
            if (uColor_ != -1) glUniform3f(uColor_, 0.25f, 0.6f, 0.85f);

            cube_.draw();
        }

        //2D passes
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

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
            else if (auto* pong = dynamic_cast<PongScene*>(scene_.get()))
            {
                pong->renderText(spriteBatch_, uiFont_);
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

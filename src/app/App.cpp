#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>
#include <glm/vec4.hpp>
#include <chrono>
#include <algorithm> 
#include "gfx/GLDebug.hpp"
#include "core/aabb.hpp"
#include <imgui.h>
#include "app/QuickActions.hpp"
#include "app/FrameInputBuilder.hpp"
#include "app/RenderPipeline.hpp"
#include "app/Picking.hpp"
#include "scenes/ECSTestScene.hpp"


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
    if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        quick::handleKey(*app, key, action);
}

void App::onScroll(GLFWwindow* win, double, double yoff)
{
    if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
    {
        // SHIFT + wheel used to dolly 3D; leave it to ECS controllers later.
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        {
            /* no-op for now (Fly/Orbit controller will consume input) */
        }
        else if (app->scene_)
        {
            app->scene_->uiCamera().zoomBy(static_cast<float>(yoff), 1.2f);
        }
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
        auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
        if (!app) return;

        double sx;
        double sy;

        glfwGetCursorPos(win, &sx, &sy);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
        {
            const bool ctrl = (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
                (glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
            double sx, sy; glfwGetCursorPos(win, &sx, &sy);
            if (ctrl) { picking::handle3D(*app, sx, sy, app->useBVH_); return; }
            auto world = app->scene_->uiCamera().screenToWorld(sx, sy);
            std::printf("Pick world: (%.3f, %.3f)\n", world.x, world.y);
        }
    }
}

void App::onCursorPos(GLFWwindow* win, double x, double y) 
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
    if (!app) return;

    double dx = x - app->lastX_, dy = y - app->lastY_;

    if (app->orbiting3D_)
    {
        /* no-op for now (ECS Orbit/Fly controller will do mouse-look) */
    }
    else if (app->panning_) 
    {
        if (app->scene_) app->scene_->uiCamera().panPixels((float)dx, (float)dy);
    }

    app->lastX_ = x; app->lastY_ = y;
}

bool App::loadOBJIntoActive(const std::string& path, float uniformScale)
{
    std::string err;
    core::MeshData md;

    if (!core::io::loadOBJ(path, md, &err))
    {
        std::fprintf(stderr, "[OBJ] Load failed: $s\n", err.c_str());
        return false;
    }

    if (md.positions.empty() || md.indices.empty())
    {
        std::fprintf(stderr, "[OBJ] Empty mesh: %s\n", path.c_str());
    }

    //uniform scale
    if (uniformScale != 1.0f)
    {
        for (size_t i = 0; i < md.positions.size(); i++)
        {
            md.positions[i] *= uniformScale;
        }
        md.aabbModel = core::computeAABB(md.positions.data(), md.vertexCount());
    }

    loadedCPU_ = std::move(md);
    loadedModel_ = glm::mat4(1.0f);
    loadedAABBModel_ = loadedCPU_.aabbModel;
    loadedAABBWorld_ = core::transformAABB(loadedAABBModel_, loadedModel_);

    if (!loadedGL_.upload(loadedCPU_)) return false;

    //build BVH for pickin - model space
    loadedBVH_ = core::buildBVH(loadedCPU_.positions.data(), loadedCPU_.indices.data(), loadedCPU_.triCount(), 4);
    
    hasLoadedMesh_ = true;

    std::printf("[OBJ] Loaded: %zu verts, %zu tris. BVH nodes=%zu\n",
        loadedCPU_.vertexCount(), loadedCPU_.triCount(), loadedBVH_.nodes.size());

    return true;
}
DebugLines3D& App::triLines() { return triLines_; }
const DebugLines3D& App::triLines() const { return triLines_; }

DebugCross3D& App::hitCross() { return hitCross_; }
const DebugCross3D& App::hitCross() const { return hitCross_; }


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
    GLDebug::enable(false);

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

    //load 3D shader + uniforms
    if (!basic3D_.loadFromFiles("shaders/basic3d.vert", "shaders/basic3d.frag")) return false;
    uMVP_ = basic3D_.uniformLocation("u_MVP");
    uColor_ = basic3D_.uniformLocation("u_Color");

    if (!cube_.initColoredCube()) return false;

    //model kept in one place
    cubeModel_ = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));

    if (!triLines_.init()) return false;
    if (!box_.init()) return false;
    if (!hitCross_.init()) return false;

    //compute model-space AABB from CPU positions = 8 vertices
    {
        const float* pos = cube_.cpuPositions();

        if (pos)
        {
            cubeAABBModel_ = core::computeAABB(pos, 8);
            cubeAABBWorld_ = core::transformAABB(cubeAABBModel_, cubeModel_);
            box_.set(cubeAABBWorld_.min, cubeAABBWorld_.max);
        }
    }

    //Build BVH for the cube mesh once - model space
    {
        const float* pos = cube_.cpuPositions();
        const unsigned* idx = cube_.cpuIndices();
        const std::size_t tris = cube_.triCount();

        bvh_ = core::buildBVH(pos, idx, tris, /*leafMaxTris*/4);
        bvhNodes_ = (int)bvh_.nodes.size();
        std::printf("[BVH] build: nodes=%d, tris=%zu, leafMax=4\n", bvhNodes_, tris);
    }

    // Initial framebuffer size
    glfwGetFramebufferSize(window_, &fbw_, &fbh_);
    glViewport(0, 0, fbw_, fbh_);
    // Create current scene
    scene_ = std::make_unique<ECSTestScene>();

    if (!imgui_.init(window_)) return false;
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
            if (scene_) scene_->onFramebufferResize(fbw_, fbh_);
        }

        //Inputs
        static FrameInputBuilder inputBuilder;
        FrameInput in = inputBuilder.poll(window_);

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

        //FPS = simple EMA every ~0.25s
        fpsAccum_ += frameDt;
        ++fpsFrames_;

        if (fpsAccum_ >= 0.25f)
        {
            fps_ = float(fpsFrames_ / fpsAccum_);
            fpsFrames_ = 0;
            fpsAccum_ = 0.0;
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT);

        if (showImGui_) imgui_.begin();

        // 3D
        renderpipe::draw3D(*this);
        // 2D
        renderpipe::draw2D(*this);
        
        glfwSwapBuffers(window_);
    }
}

App::~App() 
{
    spriteBatch_.shutdown();
    imgui_.shutdown();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

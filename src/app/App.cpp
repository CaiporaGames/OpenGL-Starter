#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>
#include <glm/vec4.hpp>
#include <chrono>
#include <algorithm> 
#include "gfx/GLDebug.hpp"
#include "core/aabb.hpp"

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
        auto* app = static_cast<App*>(glfwGetWindowUserPointer(win));
        if (!app) return;

        const bool ctrl =
            (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
            (glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

        double sx;
        double sy;
        glfwGetCursorPos(win, &sx, &sy);

        if (ctrl)
        {
            //3D pick
            const glm::mat4 invVP = glm::inverse(app->cam3D_.vp());
            core::Ray rayW = core::screenRayFromInvVP(invVP, sx, sy, app->fbw_, app->fbh_);
            
            //Transform ray to model space
            const glm::mat4 invM = glm::inverse(app->cubeModel_);
            core::Ray rM;
            rM.origin = glm::vec3(invM * glm::vec4(rayW.origin, 1.0));
            rM.dir = glm::normalize(glm::vec3(invM * glm::vec4(rayW.dir, 0.0)));

            //Raycast
            core::RayHit hit;
            const float* pos = app->cube_.cpuPositions();
            const unsigned* idx = app->cube_.cpuIndices();
            const std::size_t tris = app->cube_.triCount();

            if (pos && idx && core::raycastMesh(rM, pos, idx, tris, hit))
            {
                const unsigned i0 = idx[3 * hit.triIndex + 0];
                const unsigned i1 = idx[3 * hit.triIndex + 1];
                const unsigned i2 = idx[3 * hit.triIndex + 2];

                auto P = [&](unsigned vi)
                {
                    return glm::vec3(pos[3 * vi + 0], pos[3 * vi + 1], pos[3 * vi + 2]);
                };

                //Triangle vertices in MODEL space
                const glm::vec3 v0 = P(i0);
                const glm::vec3 v1 = P(i1);
                const glm::vec3 v2 = P(i2);

                //Barycentric hit point in MODEL space
                const float w = 1.0f - hit.u - hit.v;
                const glm::vec3 pM = w * v0 + hit.u * v1 + hit.v * v2;

                //Back to world for display
                const glm::vec3 aW = glm::vec3(app->cubeModel_ * glm::vec4(P(i0), 1.0));
                const glm::vec3 bW = glm::vec3(app->cubeModel_ * glm::vec4(P(i1), 1.0));
                const glm::vec3 cW = glm::vec3(app->cubeModel_ * glm::vec4(P(i2), 1.0));
                const glm::vec3 pW = glm::vec3(app->cubeModel_ * glm::vec4(pM, 1.0));
                
                //outline and cross marker
                app->triLines_.setTriangle(aW, bW, cW);
                app->hitCross_.setCross(pW, /*halfLenWorld*/0.15f);

                app->pickHasHit_ = true;

                std::printf("Hit tri %d t=%.3f (u=%.3f, v=%.3f)\n", hit.triIndex, hit.t, hit.u, hit.v);
            }
            else
            {
                app->triLines_.clear();
                app->hitCross_.clear();
                app->pickHasHit_ = false;
                std::printf("No hit\n");
            }
            return; //do not do 2D pick when CTRL is down
        }
        
        //2D pick
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

        //FPS = simple EMA every ~0.25s
        fpsAccum_ += frameDt;
        ++fpsFrames_;

        if (fpsAccum_ >= 0.25f)
        {
            fps_ = float(fpsFrames_ / fpsAccum_);
            fpsFrames_ = 0;
            fpsAccum_ = 0.0;
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

        //Frustum culling per object
        {
            const core::Frustum fr = core::extractFrustum(cam3D_.vp());

            //cubeAABBWorld was computed at init from cubeModel; recompute only if the model changes
            cubeVisible_ = !core::aabbOutsideFrustum(fr, cubeAABBWorld_);
            objTotal_ = 1;
            objVisible_ = cubeVisible_ ? 1 : 0;
        }

        //3D pass, one draw call
        {
            basic3D_.use();
            if (cubeVisible_)
            {
                const glm::mat4 MVP = cam3D_.vp() * cubeModel_;
        
                if (uMVP_ != -1) glUniformMatrix4fv(uMVP_, 1, GL_FALSE, &MVP[0][0]);
                if (uColor_ != -1) glUniform3f(uColor_, 0.25f, 0.6f, 0.85f);

                cube_.draw();

                //hit tri if any
                if (pickHasHit_)
                {
                    //reuse shader; lines are already in world space
                    const glm::mat4 MVPw = cam3D_.vp();//lines in world space
                    if (uMVP_ != -1)
                    {
                        glUniformMatrix4fv(uMVP_, 1, GL_FALSE, &MVPw[0][0]);
                    }

                    if (uColor_ != -1)
                    {
                        glUniform3f(uColor_, 1.0f, 0.95f, 0.2f);
                    }

                    triLines_.draw();//extra draw only when hit exists

                    if (uColor_ != -1)
                    {
                        glUniform3f(uColor_, 1.0f, 0.25f, 1.0f);
                    }

                    hitCross_.draw();//+ 1 draw
                }

                //AABB box in world space
                {
                    const glm::mat4 MVPw = cam3D_.vp();
                    if (uMVP_ != -1)
                    {
                        glUniformMatrix4fv(uMVP_, 1, GL_FALSE, &MVPw[0][0]);
                    }
                    if (uColor_ != -1)
                    {
                        glUniform3f(uColor_, 0.9f, 0.9f, 0.9f);
                    }
                    box_.draw(); //+ 1 draw
                }
            }
           
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

            //OVERLAY in pixels
            {
                spriteBatch_.begin(fbw_, fbh_);//pixel ortho
                spriteBatch_.setTexture(uiFont_.text);
                spriteBatch_.setSampleMode(2); //png alpha

                //compose text
                char buf[256];
                const int verts = 8;
                const int tris = 12;

                std::snprintf(buf, sizeof(buf),
                    "FPS: %.1f\nVerts: %d  Tris: %d\nVisible: %d / %d\nAABB min: [%.2f %.2f %.2f]\nAABB max: [%.2f %.2f %.2f]",
                    fps_, verts, tris,
                    objVisible_, objTotal_,
                    cubeAABBWorld_.min.x, cubeAABBWorld_.min.y, cubeAABBWorld_.min.z,
                    cubeAABBWorld_.max.x, cubeAABBWorld_.max.y, cubeAABBWorld_.max.z);


                glm::vec2 glyph = { 8.0f, 12.0f };//pixel size per glyph on screen
                glm::vec4 col = { 0.95f, 0.95f, 0.95f, 1.0f };
                float pad = 8.0f;

                //bottom-left achor for first line = top-left screen with pixel ortho
                glm::vec2 bl = { pad, float(fbh_) - pad - glyph.y };
                uiFont_.drawTextBL(spriteBatch_, buf, bl, glyph, col, /*letter*/1.0f, /*line*/2.0f);

                spriteBatch_.endAndDraw(); //+ 1 draw
            }
        }

        glfwSwapBuffers(window_);
    }
}

App::~App() {
    spriteBatch_.shutdown();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>
#include <glm/vec4.hpp>
#include <chrono>
#include <algorithm> 
#include "gfx/GLDebug.hpp"
#include "core/aabb.hpp"
#include <imgui.h>

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
    if (action == GLFW_PRESS && key == GLFW_KEY_F1)
    {
        if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        {
            app->showImGui_ = !app->showImGui_;
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_F12)
    {
        if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        {
            app->wantScreenshot_ = true;
            char name[128];
            std::snprintf(name, sizeof(name), "screenshot_%dx%d.tga", app->fbw_, app->fbh_);

            if (Screenshot::saveTGA(name, app->fbw_, app->fbh_))
            {
                std::printf("[Screenshot] saved %s\n", name);
            }
            else
            {
                std::printf("[Screenshot] failed\n");
            }
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_B)
    {
        if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        {
            app->useBVH_ = !app->useBVH_;
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_W)
    {
        if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        {
            app->wireframe_ = !app->wireframe_;
            glPolygonMode(GL_FRONT_AND_BACK, app->wireframe_ ? GL_LINE : GL_FILL);
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_V)
    {
        if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        {
            app->vsyncOn_ = !app->vsyncOn_;
            glfwSwapInterval(app->vsyncOn_ ? 1 : 0);
        }
    }
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(win, GLFW_TRUE);

    if (action == GLFW_PRESS && key == GLFW_KEY_O)
    {
        if (auto* app = static_cast<App*>(glfwGetWindowUserPointer(win)))
        {
            //put a file here to try; if missing, we keep the cube.
            const char* path = "assets/models/suzanne.obj";

            if (!app->loadOBJIntoActive(path, /*uniformScale*/1.0f))
            {
                std::fprintf(stderr, "[OBJ] Could not open '%s' (place file there).\n", path);
            }
        }
    }
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
            // --- 3D pick ---
            const glm::mat4 invVP = glm::inverse(app->cam3D_.vp());
            core::Ray rayW = core::screenRayFromInvVP(invVP, sx, sy, app->fbw_, app->fbh_);

            // Active model (loaded mesh if present, else cube)
            const glm::mat4 model = app->hasLoadedMesh_ ? app->loadedModel_ : app->cubeModel_;

            // Transform ray to *model* space
            const glm::mat4 invM = glm::inverse(model);
            core::Ray rM;
            rM.origin = glm::vec3(invM * glm::vec4(rayW.origin, 1.0));
            rM.dir = glm::normalize(glm::vec3(invM * glm::vec4(rayW.dir, 0.0)));

            // Active mesh CPU data
            const float* pos = app->hasLoadedMesh_ ? app->loadedGL_.positions() : app->cube_.cpuPositions();
            const unsigned* idx = app->hasLoadedMesh_ ? app->loadedGL_.indices() : app->cube_.cpuIndices();
            const std::size_t tris = app->hasLoadedMesh_ ? app->loadedGL_.triCount() : app->cube_.triCount();

            // Select the correct BVH for the active mesh
            const core::BVH& bvh = app->hasLoadedMesh_ ? app->loadedBVH_ : app->bvh_;

            core::RayHit hitBVH{}, hitBrute{};
            bool okBVH = false, okBrute = false;

            // BVH raycast (with timing/stats)
            core::BVHStats st{};
            auto t0 = std::chrono::high_resolution_clock::now();
            okBVH = core::raycastBVH(rM, pos, bvh, hitBVH, &st);
            auto t1 = std::chrono::high_resolution_clock::now();
            double msBVH = std::chrono::duration<double, std::micro>(t1 - t0).count() / 1000.0;

            // Brute force (optional compare)
            t0 = std::chrono::high_resolution_clock::now();
            okBrute = core::raycastMesh(rM, pos, idx, tris, hitBrute);
            t1 = std::chrono::high_resolution_clock::now();
            double msBrute = std::chrono::duration<double, std::micro>(t1 - t0).count() / 1000.0;

            // Decide which result to use
            const bool useBVH = app->useBVH_;
            const bool ok = useBVH ? okBVH : okBrute;
            const core::RayHit& hit = useBVH ? hitBVH : hitBrute;

            if (ok)
            {
                // Fetch triangle vertex indices from the correct source
                unsigned i0, i1, i2;
                if (useBVH) {
                    const unsigned base = 3u * hit.triIndex; // triIndex is into the BVH's *reordered* list
                    i0 = bvh.indices[base + 0];
                    i1 = bvh.indices[base + 1];
                    i2 = bvh.indices[base + 2];
                }
                else {
                    i0 = idx[3u * hit.triIndex + 0];
                    i1 = idx[3u * hit.triIndex + 1];
                    i2 = idx[3u * hit.triIndex + 2];
                }

                auto P = [&](unsigned vi) { return glm::vec3(pos[3 * vi + 0], pos[3 * vi + 1], pos[3 * vi + 2]); };

                // Model-space verts
                const glm::vec3 v0 = P(i0), v1 = P(i1), v2 = P(i2);

                // Hit point (barycentric) in model space
                const float w = 1.0f - hit.u - hit.v;
                const glm::vec3 pM = w * v0 + hit.u * v1 + hit.v * v2;

                //parametric point along the ray
                //const glm::vec3 pM_t = rM.origin + hit.t * rM.dir;

                // To world for drawing (use active model, not always cubeModel_)
                const glm::vec3 aW = glm::vec3(model * glm::vec4(v0, 1));
                const glm::vec3 bW = glm::vec3(model * glm::vec4(v1, 1));
                const glm::vec3 cW = glm::vec3(model * glm::vec4(v2, 1));
                const glm::vec3 pW = glm::vec3(model * glm::vec4(pM, 1));

                app->triLines_.setTriangle(aW, bW, cW);
                app->hitCross_.setCross(pW, 0.15f);
                app->pickHasHit_ = true;

                std::printf("Pick tri=%d t=%.3f (u=%.3f,v=%.3f)  BVH=%.3f ms (nodes=%d visited=%d triTests=%d) | brute=%.3f ms\n",
                    hit.triIndex, hit.t, hit.u, hit.v, msBVH,
                    (int)bvh.nodes.size(), st.nodesVisited, st.triTests, msBrute);
            }
            else
            {
                app->triLines_.clear();
                app->hitCross_.clear();
                app->pickHasHit_ = false;
                std::printf("Pick: none (BVH=%.3f ms, brute=%.3f ms)\n", msBVH, msBrute);
            }

            return; // do not do 2D pick when CTRL is down
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
    cam3D_.setViewport(fbw_, fbh_);
    // Create current scene
    scene_ = std::make_unique<MenuScene>();

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
            objTotal_ = 1;

            if (hasLoadedMesh_)
            {
                cubeVisible_ = !core::aabbOutsideFrustum(fr, loadedAABBWorld_);
            }
            else
            {
                cubeVisible_ = !core::aabbOutsideFrustum(fr, cubeAABBWorld_);
            }

            objVisible_ = cubeVisible_ ? 1 : 0;
        }

        if (showImGui_) imgui_.begin();

        //3D pass, one draw call
        {
            basic3D_.use();
            if (cubeVisible_)
            {
                const glm::mat4 model = hasLoadedMesh_ ? loadedModel_ : cubeModel_;
                const glm::mat4 MVP = cam3D_.vp() * model;
        
                if (uMVP_ != -1) glUniformMatrix4fv(uMVP_, 1, GL_FALSE, &MVP[0][0]);
                if (uColor_ != -1) glUniform3f(uColor_, 0.25f, 0.6f, 0.85f);

                if (hasLoadedMesh_)
                {
                    loadedGL_.draw();
                }
                else
                {
                    cube_.draw();
                }

                //hit tri if any
                if (pickHasHit_ && showHitViz_)
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
                if(showAABB_)
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

                const bool AM = hasLoadedMesh_;
                const std::size_t vtx = AM ? loadedGL_.vertexCount() : 8;
                const std::size_t tri = AM ? loadedGL_.triCount() : 12;
                std::snprintf(buf, sizeof(buf),
                    "FPS: %.1f\nVerts: %zu  Tris: %zu\nVisible: %d / %d\nBVH nodes: %d\nAABB min: [%.2f %.2f %.2f]\nAABB max: [%.2f %.2f %.2f]",
                    fps_, vtx, tri, objVisible_, objTotal_, AM ? (int)loadedBVH_.nodes.size() : bvhNodes_,
                    (AM ? loadedAABBWorld_.min.x : loadedAABBWorld_.min.x), (AM ? loadedAABBWorld_.min.y : loadedAABBWorld_.min.y), (AM ? loadedAABBWorld_.min.z : loadedAABBWorld_.min.z),
                    (AM ? loadedAABBWorld_.max.x : loadedAABBWorld_.max.x), (AM ? loadedAABBWorld_.max.y : loadedAABBWorld_.max.y), (AM ? loadedAABBWorld_.max.z : loadedAABBWorld_.max.z));



                glm::vec2 glyph = { 8.0f, 12.0f };//pixel size per glyph on screen
                glm::vec4 col = { 0.95f, 0.95f, 0.95f, 1.0f };
                float pad = 8.0f;

                //bottom-left achor for first line = top-left screen with pixel ortho
                glm::vec2 bl = { pad, float(fbh_) - pad - glyph.y };
                uiFont_.drawTextBL(spriteBatch_, buf, bl, glyph, col, /*letter*/1.0f, /*line*/2.0f);

                spriteBatch_.endAndDraw(); //+ 1 draw
            }
        }
        if (showImGui_)
        {
            //degub windows
            if (ImGui::Begin("Debug"))
            {
                ImGui::Text("FPS: %0.1f", fps_);

                //push into ring
                fpsHistory_[fpsHead_] = fps_;
                fpsHead_ = (fpsHead_ + 1) % 120;
                ImGui::PlotLines("fps", fpsHistory_, 120, fpsHead_, nullptr, 0.0f, 240.0f, ImVec2(0, 60));

                ImGui::SeparatorText("Render");
                ImGui::Checkbox("VSync (V)", &vsyncOn_);

                if (ImGui::IsItemEdited()) glfwSwapInterval(vsyncOn_ ? 1 : 0);

                ImGui::Checkbox("Wireframe (W)", &wireframe_);

                if (ImGui::IsItemEdited())
                {
                    glPolygonMode(GL_FRONT_AND_BACK, wireframe_ ? GL_LINE : GL_FILL);
                }

                ImGui::Checkbox("Show AABB", &showAABB_);
                ImGui::Checkbox("Shot Hit viz", &showHitViz_);

                ImGui::SeparatorText("Picking");
                ImGui::Checkbox("Use BVH (B)", &useBVH_);
                ImGui::Text("Has hit: %s", pickHasHit_ ? "yes" : "no");

                ImGui::SeparatorText("Camera");
                auto p = cam3D_.position();
                ImGui::Text("Post: (%0.2f, %0.2f, %0.2f)", p.x, p.y, p.z);

                if (wantScreenshot_)
                {
                    char name[128];
                    std::snprintf(name, sizeof(name), "screenshot_%dx%d.tga", fbw_, fbh_);
                   
                    if (Screenshot::saveTGA(name, fbw_, fbh_))
                        std::printf("[Screenshot] saved %s\n", name);
                    else
                        std::printf("[Screenshot] failed\n");
                    wantScreenshot_ = false;
                }
            }
            ImGui::End();

            //GL debug log
            if (ImGui::Begin("GL Log"))
            {
                const auto& msgs = GLDebug::messages();
                ImGui::BeginChild("glmsg", ImVec2(0,0), true);

                for (const auto& m : msgs)
                {
                    ImGui::Text(
                    "[%s] id=%u %s", 
                        m.severity==GL_DEBUG_SEVERITY_HIGH ? "HIGH" : 
                        m.severity==GL_DEBUG_SEVERITY_MEDIUM ? "MEDIUM" : 
                        m.severity==GL_DEBUG_SEVERITY_LOW ? "LOW" : "NOTE",
                        m.id, m.text.c_str());
                }
                ImGui::EndChild();
                
                if (ImGui::Button("Clear")) GLDebug::clear();
            }

            ImGui::End();
            imgui_.end();
        }
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

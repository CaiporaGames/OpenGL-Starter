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
#include "core/camera/OrbitCamera3D.hpp"
#include "render/Mesh3D.hpp"
#include "render/DebugLines3D.hpp"
#include "render/DebugCross3D.hpp"
#include "core/raycast.hpp"
#include "core/camera/ScreenRay.hpp"
#include "render/BoxWire3D.hpp"
#include "core/aabb.hpp"
#include "core/frustum.hpp"
#include "core/bvh.hpp"
#include "core/mesh_data.hpp"
#include "core/io/obj_loader.hpp"
#include "render/MeshGL.hpp"
#include "ui/ImGuiLayer.hpp"
#include "gfx/GLDebug.hpp"
#include "util/Screenshot.hpp"

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
    GLFWwindow* windowHandle() const { return window_; }
    int fbw() const { return fbw_; }
    int fbh() const { return fbh_; }
    bool showImGui() const { return showImGui_; }
    void setShowImGui(bool value) { showImGui_ = value; } 
    void setWantScreenshot(bool value) { wantScreenshot_ = value; }  
    void setUseBVH(bool value) { useBVH_ = value; }  
    bool& useBVH() { return useBVH_; }
    void setWireframe(bool value) { wireframe_ = value; }
    bool& wireframe() { return wireframe_; }
    void setVsyncOn(bool value) { vsyncOn_ = value; }
    bool& vsyncOn() { return vsyncOn_; }
    const IScene* scene() const { return scene_.get(); }
    SpriteBatch& spriteBatch() { return spriteBatch_; }
    const BitmapFont& uiFont() const { return uiFont_; }
    const core::AABB& loadedAABBWorld() { return loadedAABBWorld_; };
    const core::AABB& loadedAABBModel() { return loadedAABBModel_; };
    const float fps() { return fps_; }
    const int objVisible() { return objVisible_; }
    void setObjVisible(int value) { objVisible_ = value; }
    const bool cubeVisible() { return cubeVisible_; }
    void setCubeVisible(bool value) { cubeVisible_ = value; }
    void setCubeAABBWorld(core::AABB& value) { cubeAABBWorld_ = value; }
    const core::AABB& cubeAABBWorld() { return cubeAABBWorld_; }
    const glm::mat4& cubeModel() { return cubeModel_; }
    const bool hasLoadedMesh() { return hasLoadedMesh_; }
    const glm::mat4& loadedModel() { return loadedModel_; }
    bool& showAABB() { return showAABB_; }
    void setShowAABB(bool value) { showAABB_ = value; }
    const bool pickHasHit() { return pickHasHit_; }
    void setPickHasHit(bool value) { pickHasHit_ = value; }
    bool& showHitViz() { return showHitViz_; }
    void setShowHitViz(bool value) { showHitViz_ = value; }
    const MeshGL& loadedGL() { return loadedGL_; }
    const Mesh3D& cube() { return cube_; };
    const ShaderProgram& basic3D() { return basic3D_; }
    const GLint& uMVP() { return uMVP_; }
    ImGuiLayer& imgui() { return imgui_; }
    const int fpsHead() { return fpsHead_; }
    void setFpsHead(int value) { fpsHead_ = value; }
    float* fpsHistory() { return fpsHistory_; }
    bool wantScreenshot() { return wantScreenshot_; }
    void clearScreenshotRequest() { wantScreenshot_ = false; }
    const GLint& uColor() { return uColor_; }
    DebugLines3D& triLines();                  // non-const overload
    const DebugLines3D& triLines() const;      // const overload
    DebugCross3D& hitCross();                  // non-const overload
    const DebugCross3D& hitCross() const;      // const overload
    const BoxWire3D& box() {return box_;}
    int& objTotal() { return objTotal_; }
    void setObjTotal(int value) { objTotal_ = value; }
    const int bvhNodes() { return bvhNodes_; }
    const core::BVH& loadedBVH() { return loadedBVH_; }
    bool loadOBJIntoActive(const std::string& path, float uniformScale = 1.0f);
    MeshGL& unitCubeMesh() { return loadedGL_; }
    const core::BVH& bvh() { return bvh_; }

private:
    GLFWwindow* window_ = nullptr;
    SpriteBatch spriteBatch_;
    std::unique_ptr<IScene> scene_;   // <— host ANY scene

    Texture2D fontTex_;
    BitmapFont uiFont_;
    GLuint whiteTex_ = 0;
    // fixed-step accumulator
    double acc_ = 0.0;
    int fbw_ = 0;
    int fbh_ = 0;
    bool prevMouseLeftDown_ = false;

    // pan state for MMB drag
    bool panning_ = false;
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

    //BVH
    core::BVH bvh_;
    int bvhNodes_ = 0;
    bool useBVH_ = true; //toggle if you want to compare quickly

    //active mesh: either cube_ or loadedMesh_
    bool hasLoadedMesh_ = false;
    core::MeshData loadedCPU_;
    MeshGL loadedGL_;
    glm::mat4 loadedModel_{ 1.0f };
    core::BVH loadedBVH_;
    core::AABB loadedAABBModel_{};
    core::AABB loadedAABBWorld_{};

    ImGuiLayer imgui_;
    bool showImGui_ = true;
    bool showAABB_ = true;
    bool showHitViz_ = true;
    bool wireframe_ = false;
    bool vsyncOn_ = true;
    bool wantScreenshot_ = false;

    //Tiny rolling FPS graph - last 120 frames
    float fpsHistory_[120] = { 0.0f };
    int fpsHead_ = 0;
};

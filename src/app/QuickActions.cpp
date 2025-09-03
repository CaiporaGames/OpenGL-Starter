#include "app/QuickActions.hpp"
#include "app/App.hpp"
#include "util/Screenshot.hpp"
#include <GLFW/glfw3.h>
#include <cstdio>

namespace quick 
{

    static void saveScreenshot(App& app) 
    {
        char name[128];
        std::snprintf(name, sizeof(name), "screenshot_%dx%d.tga", app.fbw(), app.fbh());
        if (Screenshot::saveTGA(name, app.fbw(), app.fbh()))
            std::printf("[Screenshot] saved %s\n", name);
        else
            std::printf("[Screenshot] failed\n");
    }

    void handleKey(App& app, int key, int action)
    {
        if (action != GLFW_PRESS) return;

        switch (key)
        {
        case GLFW_KEY_F1:
            app.setShowImGui(!app.showImGui());
            break;
        case GLFW_KEY_F12:
            app.setWantScreenshot(true); // defer to UI, or save immediately:
            saveScreenshot(app);
            app.setWantScreenshot(false);
            break;
        case GLFW_KEY_B:
            app.setUseBVH(!app.useBVH());
            break;
        case GLFW_KEY_W:
            app.setWireframe(!app.wireframe());
            glPolygonMode(GL_FRONT_AND_BACK, app.wireframe() ? GL_LINE : GL_FILL);
            break;
        case GLFW_KEY_V:
            app.setVsyncOn(!app.vsyncOn());
            glfwSwapInterval(app.vsyncOn() ? 1 : 0);
            break;
        case GLFW_KEY_O:
        {
            const char* path = "assets/models/suzanne.obj";
            if (!app.loadOBJIntoActive(path, 1.0f))
                std::fprintf(stderr, "[OBJ] Could not open '%s'.\n", path);
        }
        break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(app.windowHandle(), GLFW_TRUE);
            break;
        default: break;
        }
    }

} // namespace quick

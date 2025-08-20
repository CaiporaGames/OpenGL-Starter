#include "app/App.hpp"
#include <cstdio>
#include <cstdlib>

void App::onError(int code, const char* desc) {
    std::fprintf(stderr, "[GLFW %d] %s\n", code, desc);
}
void App::onKey(GLFWwindow* win, int key, int, int action, int) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(win, GLFW_TRUE);
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

    // Center window (optional)
    GLFWmonitor* mon = glfwGetPrimaryMonitor();
    int ax, ay, aw, ah; glfwGetMonitorWorkarea(mon, &ax, &ay, &aw, &ah);
    int ww, wh; glfwGetWindowSize(window_, &ww, &wh);
    glfwSetWindowPos(window_, ax + (aw - ww) / 2, ay + (ah - wh) / 2);

    // init renderer (loads shaders, VBO/VAO)
    if (!renderer_.init("shaders/triangle.vert", "shaders/triangle.frag"))
        return false;

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

        renderer_.draw();

        glfwSwapBuffers(window_);
    }
}

App::~App() {
    renderer_.shutdown();
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
}

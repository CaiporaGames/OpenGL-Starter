#pragma once
#include "engine/IScene.hpp"  // for FrameInput
struct GLFWwindow;

struct FrameInputBuilder {
    bool prevMouseLeftDown = false;
    double lastX = 0.0, lastY = 0.0;

    // Build FrameInput each frame from GLFW
    FrameInput poll(GLFWwindow* w) {
        FrameInput in{};

        in.keyW = glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS;
        in.keyA = glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS;
        in.keyS = glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS;
        in.keyD = glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS;
        in.keyQ = glfwGetKey(w, GLFW_KEY_Q) == GLFW_PRESS;
        in.keyE = glfwGetKey(w, GLFW_KEY_E) == GLFW_PRESS;
        in.keySpace = glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS;
        in.keyC = glfwGetKey(w, GLFW_KEY_C) == GLFW_PRESS;

        double mx, my; glfwGetCursorPos(w, &mx, &my);
        const bool lDown = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        in.mouseX = mx; in.mouseY = my;
        in.mouseLeftDown = lDown;
        in.mouseLeftPressed = lDown && !prevMouseLeftDown;
        in.mouseLeftReleased = !lDown && prevMouseLeftDown;
        prevMouseLeftDown = lDown;

        // extras that help controllers/editor later
        in.rmbDown = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        in.mmbDown = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        in.lshiftDown = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        in.lctrlDown = glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

        // deltas (optional)
        in.mouseDeltaX = mx - lastX;
        in.mouseDeltaY = my - lastY;
        lastX = mx; lastY = my;
        return in;
    }
};

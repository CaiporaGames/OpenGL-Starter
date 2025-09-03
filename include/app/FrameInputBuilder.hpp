#pragma once
#include "engine/IScene.hpp"  // for FrameInput
struct GLFWwindow;

struct FrameInputBuilder {
    bool prevMouseLeftDown = false;
    double lastX = 0.0, lastY = 0.0;

    // Build FrameInput each frame from GLFW
    FrameInput poll(GLFWwindow* w) {
        FrameInput in{};
        // WASD / arrows (keep your semantics)
        in.leftUp = glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS;
        in.leftDown = glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS;
        in.rightUp = glfwGetKey(w, GLFW_KEY_UP) == GLFW_PRESS;
        in.rightDown = glfwGetKey(w, GLFW_KEY_DOWN) == GLFW_PRESS;

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

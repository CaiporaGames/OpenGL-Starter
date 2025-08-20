#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gfx/TriangleRenderer.hpp"

class App {
public:
	App() = default;
	~App();
	bool init(int w = 800, int h = 600, const char* title = "GL App");
	void run();

private:
	GLFWwindow* window_ = nullptr;
	TriangleRenderer renderer_;

	static void onError(int code, const char* desc);
	static void onKey(GLFWwindow* win, int key, int sc, int action, int mods);
};

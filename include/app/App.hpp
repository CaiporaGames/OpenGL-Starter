#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "gfx/TriangleRenderer.hpp"
#include "gfx/SpriteBatch.hpp"  
#include "gfx/OrthoCamera2D.hpp"

class App {
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

private:
	GLFWwindow* window_ = nullptr;
	SpriteBatch renderer_;
	OrthoCamera2D camera_;
	bool   panning_ = false;
	double lastX_ = 0.0, lastY_ = 0.0;
};

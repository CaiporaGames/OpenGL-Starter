#pragma once
#include <GLFW/glfw3.h>

class ImGuiLayer
{
public:
	bool init(GLFWwindow* window);//aster context is current
	void begin();
	void end();//renders draw data
	void shutdown();
};
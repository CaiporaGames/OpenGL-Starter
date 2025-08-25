#pragma once
#include <glad/glad.h>

struct Texture2D
{
	GLuint id = 0;
	int width = 0;
	int height = 0;
	int channelAmount = 0;
	//path to the texture and set it to nearest crisp by defalt
	bool load(const char* path, bool neareast = true);
	void destroy();
};
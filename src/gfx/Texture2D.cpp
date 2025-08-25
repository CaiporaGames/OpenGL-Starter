#include "gfx/Texture2D.hpp"
#include "thirdparty/stb_image.h"
#include <iostream>

bool Texture2D::load(const char* path, bool nearest)
{
	destroy();

	stbi_set_flip_vertically_on_load(true);
	unsigned char* pixels = stbi_load(path, &width, &height, &channelAmount, 0);
	if (!pixels)
	{
		std::cout << "[Texture2D failed] " << path << std::endl;
		return false;
	}

	GLenum format = channelAmount == 4 ? GL_RGBA : channelAmount == 3 ? GL_RGB : GL_RED;

	GLint prevUnpack = 0;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevUnpack);

	if ((width * channelAmount) % 4 != 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	//crisp font default nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nearest ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nearest ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, 
		(format == GL_RGBA) ? GL_RGBA8 : (format == GL_RGB) ? GL_RGB8 : GL_R8, 
		width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	if (!nearest) glGenerateMipmap(GL_TEXTURE_2D);

	if ((width * channelAmount) % 4 != 0) glPixelStorei(GL_UNPACK_ALIGNMENT, prevUnpack);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pixels);
	return true;
}

void Texture2D::destroy()
{
	if (id) glDeleteTextures(1, &id), id = 0;
}


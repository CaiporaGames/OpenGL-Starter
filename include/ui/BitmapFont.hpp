#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "gfx/SpriteBatch.hpp"

struct BitmapFont
{
	//texture info
	unsigned int text = 0;  // GL texture id
	int textureWidth = 0;
	int textureHeight = 0;

	//grid layout
	int columns = 16;
	int rows = 16;
	int cellPixelWidth = 8;
	int cellPixelHeight = 8;
	char first = 32;
	char last = 126;

	//compute uv for single character
	// BitmapFont.hpp
	glm::vec4 uvFor(char c) const 
	{
		if (c < first || c > last) c = '?';
		int idx = int(c) - int(first);
		int cx = idx % columns;
		int cy = idx / columns;

		const float du = 1.0f / float(columns);
		const float dv = 1.0f / float(rows);

		const float u0 = cx * du;
		// stbi_set_flip_vertically_on_load(true): row 0 = TOP
		const float v0 = 1.0f - (cy + 1) * dv;

		// optional: small inset to avoid bleeding if glyphs touch cell borders
		const float insetU = 0.0f;
		const float insetV = 0.0f;

		return { u0 + insetU, v0 + insetV, u0 + du - insetU, v0 + dv - insetV };
	}


	//measure texture in world units, given a glyph world size and spacing
	glm::vec4 measure(const std::string& str_text, const glm::vec2& in_glyphWorld,
		float in_letterSpacing = 0.0f, float in_lineSpacing = 0.0f) const
	{
		glm::vec2 pen(0.0f);
		glm::vec2 size(0.0f);
		float advX = in_glyphWorld.x + in_letterSpacing;
		float advY = in_glyphWorld.y + in_lineSpacing;
		float lineWidth = 0.0f;

		for (char c : str_text)
		{
			if (c == '\n')
			{
				size.x = std::max(size.x, lineWidth);
				lineWidth = 0.0f;
				pen.x = 0.0f;
				pen.y -= advY;
				continue;
			}
			lineWidth += advX;
		}

		size.x = std::max(size.x, lineWidth);
		//height = number of lines * advY + one line
		int lines = 1 + int(std::count(str_text.begin(), str_text.end(), '\n'));
		size.y = lines * advY;
		return glm::vec4(0.0f, 0.0f, size.x, size.y);
	}

	//Draw texture with bottom-left anchor in world units
	void drawTextBL(SpriteBatch& batch, const std::string& str_text, glm::vec2 bottomLeft, const glm::vec2& in_glyphWorld,
		glm::vec4& in_color, float in_letterSpacing = 0.0f, float in_lineSpacing = 0.0f) const
	{
		glm::vec2 pen = bottomLeft;
		float advX = in_glyphWorld.x + in_letterSpacing;
		float advY = in_glyphWorld.y + in_lineSpacing;

		for (char c : str_text)
		{
			if (c == '\n')
			{
				pen.x = bottomLeft.x;
				pen.y -= advY;
				continue;
			}
			glm::vec4 uv = uvFor(c);
			Sprite s{};
			s.pos = pen;
			s.size = in_glyphWorld;
			s.uv = uv;
			s.color = in_color;
			batch.push(s);
			pen.x += advX;
		}
	}
};
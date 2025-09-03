#pragma once
#include <string>

namespace Screenshot
{
	// Saves the current framebuffer as an uncompressed 24bpp TGA.
	// Flips vertically so it opens correctly in common viewers.
	bool saveTGA(const std::string& path, int fbw, int fbh);
}
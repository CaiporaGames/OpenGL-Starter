#include "util/Screenshot.hpp"
#include <glad/glad.h>
#include <vector>
#include <cstdio>

namespace Screenshot {

    bool saveTGA(const std::string& path, int w, int h)
    {
        if (w <= 0 || h <= 0) return false;

        // Preserve state we touch
        GLint prevReadFBO = 0, prevReadBuf = 0, prevPack = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
        glGetIntegerv(GL_READ_BUFFER, &prevReadBuf);
        glGetIntegerv(GL_PACK_ALIGNMENT, &prevPack);

        // Read from default framebuffer back buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glReadBuffer(GL_BACK);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        // 1) Read RGBA from GPU
        std::vector<unsigned char> rgba((size_t)w * h * 4);
        glFinish(); // ensure all draws are done before we read
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

        // 2) Convert to 24-bit BGR and flip vertically
        std::vector<unsigned char> bgr((size_t)w * h * 3);
        for (int y = 0; y < h; ++y) {
            const int srcY = y;
            const int dstY = (h - 1 - y);
            const unsigned char* src = &rgba[(size_t)srcY * w * 4];
            unsigned char* dst = &bgr[(size_t)dstY * w * 3];
            for (int x = 0; x < w; ++x) {
                dst[3 * x + 0] = src[4 * x + 2]; // B
                dst[3 * x + 1] = src[4 * x + 1]; // G
                dst[3 * x + 2] = src[4 * x + 0]; // R
            }
        }

        // 3) Write uncompressed 24-bpp TGA (top-left origin)
        unsigned char header[18] = {};
        header[2] = 2; // true-color
        header[12] = (unsigned char)(w & 0xFF);
        header[13] = (unsigned char)((w >> 8) & 0xFF);
        header[14] = (unsigned char)(h & 0xFF);
        header[15] = (unsigned char)((h >> 8) & 0xFF);
        header[16] = 24;
        header[17] = 0x20; // top-left origin

        FILE* f = std::fopen(path.c_str(), "wb");
        if (!f) {
            // restore and bail
            glPixelStorei(GL_PACK_ALIGNMENT, prevPack);
            if (prevReadBuf) glReadBuffer(prevReadBuf);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prevReadFBO);
            return false;
        }
        std::fwrite(header, 1, 18, f);
        std::fwrite(bgr.data(), 1, bgr.size(), f);
        std::fclose(f);

        // Restore GL state
        glPixelStorei(GL_PACK_ALIGNMENT, prevPack);
        if (prevReadBuf) glReadBuffer(prevReadBuf);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)prevReadFBO);

        return true;
    }

} // namespace Screenshot

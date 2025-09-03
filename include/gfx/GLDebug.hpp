#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>
#include <cstdint>

namespace GLDebug 
{
    struct Message 
    {
        GLuint id{};
        GLenum source{};
        GLenum type{};
        GLenum severity{};
        std::string text;
    };

    void enable(bool verbose = false);      // call once after GLAD
    void disable();
    void clear();                           // clear stored messages
    const std::vector<Message>& messages(); // view ring buffer

} // namespace GLDebug

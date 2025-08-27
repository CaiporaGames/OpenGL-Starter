#pragma once
#include <glad/glad.h>

namespace GLDebug 
{
    // Enable KHR_debug / ARB_debug_output when available. Safe to call multiple times.
    void enable();
}

#version 330 core
layout(location = 0) in vec2 aPos;    // screen-space (after model) in pixels
layout(location = 1) in vec2 aUV;     // 0..1 (or atlas sub-rect)
layout(location = 2) in vec4 aColor;  // per-vertex tint

uniform mat4 u_P; // orthographic projection in pixels

out vec2 vUV;
out vec4 vColor;

void main() {
    vUV    = aUV;
    vColor = aColor;
    gl_Position = u_P * vec4(aPos, 0.0, 1.0);
}

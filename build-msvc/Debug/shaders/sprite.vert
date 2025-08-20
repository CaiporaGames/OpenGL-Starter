#version 330 core
layout(location = 0) in vec2 aPos; // model-space, unit quad [0..1]
layout(location = 1) in vec2 aUV;

uniform mat4 u_MVP;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = u_MVP * vec4(aPos, 0.0, 1.0);
}

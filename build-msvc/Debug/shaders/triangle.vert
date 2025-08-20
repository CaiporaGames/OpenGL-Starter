#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aCol;

uniform mat4 u_MVP;

out vec3 vCol;

void main() {
    vCol = aCol;
    gl_Position = u_MVP * vec4(aPos, 0.0, 1.0);
}

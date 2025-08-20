#version 330 core
in vec3 vCol;
out vec4 outColor;
void main() {
  outColor = vec4(vCol, 1.0);
}

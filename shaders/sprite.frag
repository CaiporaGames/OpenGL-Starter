#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;

void main() {
    FragColor = texture(uTex, vUV);
    // If you want tinting: FragColor *= vec4(1.0,1.0,1.0,1.0);
}

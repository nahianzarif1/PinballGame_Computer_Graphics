#version 330 core

in vec3 LightingColor;
in vec3 Color;

out vec4 FragColor;

void main() {
    FragColor = vec4(LightingColor * Color, 1.0);
}

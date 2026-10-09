#version 330 core
in vec2 UV;
out vec4 FragColor;
uniform sampler2D rayImage;
void main() { FragColor=vec4(texture(rayImage,UV).rgb,1.0); }

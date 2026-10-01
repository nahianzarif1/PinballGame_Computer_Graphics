#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform vec4 color;
uniform int useTexture;
uniform sampler2D fontAtlas;
void main() {
    if (useTexture == 1) {
        float a = texture(fontAtlas, TexCoord).r;
        if (a < 0.1) discard;
        FragColor = vec4(color.rgb, color.a * a);
    } else {
        FragColor = color;
    }
}

#version 330 core

in vec3 LightingColor;
in vec3 Color;
in float Alpha;
in vec2 TexCoord;
out vec4 FragColor;

uniform int useTexture;
uniform sampler2D textureSampler;

void main() {
    vec3 albedo = Color;
    if (useTexture == 1) {
        albedo *= texture(textureSampler, TexCoord).rgb;
    }
    FragColor = vec4(LightingColor * albedo, Alpha);
}

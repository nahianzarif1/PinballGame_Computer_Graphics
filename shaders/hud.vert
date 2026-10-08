#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
out vec2 TexCoord;
uniform mat4 projection;
uniform vec2 pos;
uniform vec2 size;
void main() {
    vec2 p = pos + aPos * size;
    TexCoord = aUV;
    gl_Position = projection * vec4(p, 0.0, 1.0);
}

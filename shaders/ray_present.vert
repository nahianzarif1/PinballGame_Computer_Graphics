#version 330 core
out vec2 UV;
void main() {
    vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);
    UV=p;
    gl_Position=vec4(p*2.0-1.0,0.0,1.0);
}

#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 o_uv;

uniform mat4 view;
uniform mat4 proj;

void main() {
    gl_Position = proj * view * vec4(pos, 1.0);
    o_uv = uv;
}

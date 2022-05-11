#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 o_frag_pos;
layout(location = 1) out vec3 o_normal;
layout(location = 2) out vec2 o_uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
    vec4 world_position = (model * vec4(pos, 1.0));
    o_frag_pos = world_position.xyz;
    o_normal = normal;
    o_uv = uv;
    gl_Position = proj * view * world_position;
}

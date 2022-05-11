#version 450 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;

uniform vec3 color;

layout(std140, binding = 0) uniform  DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
} light;

void main() {
    frag_color = vec4(color * light.color, 1.0);
}

#version 450 core

layout(location = 0) in vec3 frag_pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 0) out vec4 frag_color;

uniform vec3 color;

layout(std140, binding = 0) uniform  DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
} light;

void main() {
    vec3 norm = normalize(normal);
    float diffuse = max(dot(norm, light.direction), 0.0);
    vec3 diffuse_color = diffuse * light.color * light.intensity;
    float ambient = 0.1;
    vec3 ambient_color = ambient * light.color;

    frag_color = vec4(color * (diffuse_color + ambient_color), 1.0);
}

#version 450 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;

uniform vec3 color;

void main() {
    frag_color = vec4(color, 1.0);
}

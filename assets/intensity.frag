#version 460 core

layout(location = 0) out vec4 frag_color;
uniform vec3 color;

void main() {
    // TODO: intensity should depend on distance to camera (player)
    frag_color = vec4(color, 1.0);
}

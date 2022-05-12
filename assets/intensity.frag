#version 460 core

uniform vec3 color;

void main() {
    // TODO: intensity should depend on distance to camera (player)
    gl_FragColor = vec4(color, 1.0);
}

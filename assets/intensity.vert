#version 460 core

void main() {
    vec2 positions[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0));

    int indices[6] = int[](
    0, 1, 2,
    2, 3, 0);

    gl_Position = vec4(positions[indices[gl_VertexID]], 0.0, 1.0);
}

#version 460 core

layout(location = 0)  out vec2 o_uv;

void main() {
    vec2 positions[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0));

    vec2 uvs[4] = vec2[](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0));

    int indices[6] = int[](
    0, 1, 2,
    2, 3, 0);

    int index = indices[gl_VertexID];
    o_uv = uvs[index];
    gl_Position = vec4(positions[index], 0.0, 1.0);
}

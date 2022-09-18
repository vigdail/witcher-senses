#version 460 core

layout(location = 0) in vec2 o_uv;

layout(location = 0) out vec4 frag_color;

uniform sampler2D hdr;

void main() {
    vec3 color = texture(hdr, o_uv).rgb;
    float exposure = 1.0;
    color = vec3(1.0) - exp(-color * exposure);
    frag_color = vec4(color, 1.0);
}

#version 460 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;

uniform sampler2D intensity_map;

float getParams(vec2 uv) {
    float d = dot(uv, uv);
    d = 1.0 - d;
    d = max(d, 0.0);

    return d;
}

void main() {
    vec2 textureUv = uv * 2.0;
    vec2 flooredUv = floor(textureUv);
    vec2 uv1 = flooredUv;
    vec2 uv2 = flooredUv + vec2(-1.0, -0.0);
    vec2 uv3 = flooredUv + vec2(-0.0, -1.0);
    vec2 uv4 = flooredUv + vec2(-1.0, -1.0);

    vec4 mask;
    mask.x = getParams(uv1);
    mask.y = getParams(uv2);
    mask.z = getParams(uv3);
    mask.w = getParams(uv4);

    vec4 intensity = texture(intensity_map, textureUv);
    float masterFilter = dot(intensity, mask);
    frag_color = vec4(masterFilter * intensity);
}

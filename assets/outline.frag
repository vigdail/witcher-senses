#version 460 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;

uniform float time;
uniform sampler2D intensity_map;
uniform sampler2D outline_map;

float getParams(vec2 uv) {
    float d = dot(uv, uv);
    d = 1.0 - d;
    d = max(d, 0.0);

    return d;
}

float integerNoise(int n)
{
    n = (n >> 13) ^ n;
    int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
    return (float(nn) / 1073741824.0);
}

void main() {
    vec2 texture_uv = uv * 2.0;
    vec2 floored_uv = floor(texture_uv);
    vec2 uv1 = floored_uv;
    vec2 uv2 = floored_uv + vec2(-1.0, -0.0);
    vec2 uv3 = floored_uv + vec2(-0.0, -1.0);
    vec2 uv4 = floored_uv + vec2(-1.0, -1.0);

    vec4 mask;
    mask.x = getParams(uv1);
    mask.y = getParams(uv2);
    mask.z = getParams(uv3);
    mask.w = getParams(uv4);

    vec4 intensity = texture(intensity_map, texture_uv);
    float master_filter = dot(intensity, mask);

    float texel_size = 1.0/256.0;
    vec2 sampling1 = texture_uv + vec2(texel_size, 0.0);
    vec2 sampling2 = texture_uv + vec2(-texel_size, 0.0);
    vec2 sampling3 = texture_uv + vec2(0.0, texel_size);
    vec2 sampling4 = texture_uv + vec2(0.0, -texel_size);

    vec2 intensity_x0 = texture(intensity_map, sampling1).xy;
    vec2 intensity_x1 = texture(intensity_map, sampling2).xy;
    vec2 intensity_diff_x = intensity_x0 - intensity_x1;

    vec2 intensity_y0 = texture(intensity_map, sampling3).xy;
    vec2 intensity_y1 = texture(intensity_map, sampling4).xy;
    vec2 intensity_diff_y = intensity_y0 - intensity_y1;

    vec2 max_abs_difference = max(abs(intensity_diff_x), abs(intensity_diff_y));
    max_abs_difference = clamp(max_abs_difference, 0.0, 1.0);

    vec2 outlines = master_filter * max_abs_difference;
    vec2 last_outlines = texture(outline_map, uv).xy;

    float param_outline = master_filter * 0.15 + last_outlines.y;
    param_outline += 0.35 * outlines.r;
    param_outline += 0.35 * outlines.g;

    vec2 noise_weights = vec2(time, 0.0);
    vec2 noise_inputs = 150.0 * uv + 300.0 * noise_weights;
    ivec2 i_noise_inputs = ivec2(noise_inputs);

    float noise0 = clamp(integerNoise(i_noise_inputs.x + bitfieldReverse(i_noise_inputs.y)), -1, 1) + 0.65;// r0.y

    texel_size = 1.0 / 512.0;

    sampling1 = clamp(uv + vec2(texel_size, 0.0), 0.0, 1.0);
    sampling2 = clamp(uv + vec2(-texel_size, 0.0), 0.0, 1.0);
    sampling3 = clamp(uv + vec2(0.0, texel_size), 0.0, 1.0);
    sampling4 = clamp(uv + vec2(0.0, -texel_size), 0.0, 1.0);

    float outline_x0 = texture(outline_map, sampling1).x;
    float outline_x1 = texture(outline_map, sampling2).x;
    float outline_y0 = texture(outline_map, sampling3).x;
    float outline_y1 = texture(outline_map, sampling4).x;
    float average_outline = (outline_x0 + outline_x1 + outline_y0 + outline_y1) / 4.0;

    float frame_outline_difference = average_outline - last_outlines.x;
    frame_outline_difference *= noise0;

    float new_noise = last_outlines.x * noise0;

    float new_outline = frame_outline_difference * 0.9 + param_outline;
    new_outline -= 0.24*new_noise;

    vec2 final_outline = vec2(last_outlines.x + new_outline, new_outline);

    float damping_param = 0.5;
    damping_param = pow(damping_param, 100);
    float damping = 0.7 + 0.16 * damping_param;

    frag_color = vec4(final_outline * damping, 0.0, 1.0);
}

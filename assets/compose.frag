#version 460 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 frag_color;

uniform vec2 texture_size;
uniform float time;
uniform sampler2D color_map;
uniform sampler2D outline_map;
uniform sampler2D intensity_map;

const float PI = 3.1415;
const float PI_4 = PI / 4.0;

void main() {
    float zoom_amount = (sin(time) + 1.0) / 2.0;// 1.0;

    // Another value which affect fisheye effect
    // but always set to vec2(1.0, 1.0).
    vec2 amount = vec2(1.0, 1.0);// cb0_v2.zw;

    // Colors of witcher senses
    vec3 color_interesting = vec3(1.0, 0.8, 0.4);// cb3_v5.rgb;
    vec3 color_traces = vec3(0.9, 0.0, 0.0);// cb3_v4.rgb;

    // Was always set to vec2(0.0, 0.0).
    // Setting this to higher values
    // makes "grey corners" effect weaker.
    vec2 offset = vec2(0.0, 0.0);// cb3_v2.xy;

    // Main value which causes fisheye effect [0-1]
    const float fisheye_amount = zoom_amount;// 1.0;

    // Scale at first from [0-1] to [-1;1], then calculate abs
    vec2 uv3 = abs(uv * 2.0 - 1.0);

    // Aspect ratio
    float aspect_ratio = texture_size.x / texture_size.y;

    // * Mask used to make corners grey
    float mask_gray_corners;
    {
        vec2 new_uv = vec2(uv3.x * aspect_ratio, uv3.y) - offset;
        new_uv = clamp(new_uv / 1.8, 0.0, 1.0);
        new_uv = pow(new_uv, vec2(2.5));

        mask_gray_corners = 1-min(1.0, length(new_uv));
    }

    // circle radius used further
    float circle_radius;
    {
        vec2 corners0 = clamp(vec2(0.03, 0.03) - uv, vec2(0.0), vec2(1.0));
        float cor = corners0.x + corners0.y;

        vec2 corners1 = clamp(uv - vec2(0.97, 0.97), vec2(0.0), vec2(1.0));

        cor += corners1.x;
        cor += corners1.y;

        circle_radius = clamp(cor * 20.0, 0.0, 1.0);// r0.x, line 21
    }

    // * Zooming effect
    vec2 offset_uv = vec2(0.0);
    vec2 color_uv = vec2(0.0);
    {
        vec2 uv4  = 2 * uv;
        uv4 -= vec2(1.0, 1.0);

        float mask3  = dot(uv4, uv4);
        uv4 *= mask3;

        float attenuation = fisheye_amount * 0.1;
        uv4 *= attenuation;
        frag_color = vec4(uv4, 0.0, 1.0);

        offset_uv = clamp(uv4, vec2(-0.4, -0.4), vec2(0.4, 0.4));
        offset_uv *= zoom_amount;

        color_uv = uv - offset_uv * amount;
    }
    vec3 color = texture(color_map, color_uv).rgb;

    vec2 outline_uv = color_uv * 0.5;
    float outline_interesting = texture(outline_map, outline_uv).x / 8.0;

    outline_uv += vec2(0.5, 0.0);
    float outline_traces = texture(outline_map, outline_uv).x / 8.0;

    float timeParam = time * 0.1;

    circle_radius = 1.0 - circle_radius;
    circle_radius *= 0.03;

    vec3 color_circle_main = vec3(0.0, 0.0, 0.0);
    for (int i=0; 8 > i; i++)
    {
        // full 2*PI = 360 angles cycle
        const float angleRadians = float(i) * PI_4 - timeParam;

        vec2 unit_circle;
        unit_circle.x = cos(angleRadians);
        unit_circle.y = sin(angleRadians);

        unit_circle *= circle_radius;

        vec2 uv_outline_base = color_uv + unit_circle / 8.0;

        vec2 uv_outline_interesting_circle = uv_outline_base * 0.5;
        float outline_interesting_circle  = texture(outline_map, uv_outline_interesting_circle).x;
        outline_interesting += outline_interesting_circle / 8.0;

        vec2 uv_outline_traces_circle =  uv_outline_base * 0.5 + vec2(0.5, 0.0);
        float outline_traces_circle  = texture(outline_map, uv_outline_traces_circle).x;
        outline_traces += outline_traces_circle / 8.0;

        vec2 uv_color_circle  = color_uv + unit_circle * offset_uv;
        vec3 color_circle = texture(color_map, uv_color_circle).rgb;
        color_circle_main += color_circle / 8.0;
    }

    vec2 intensity = texture(intensity_map, color_uv).xy;

    float intensityInteresting = intensity.r;
    float intensityTraces = intensity.g;

    float main_outline_interesting = clamp(outline_interesting - 0.8*intensityInteresting, 0.0, 1.0);
    float main_outline_traces = clamp(outline_traces - 0.75*intensityTraces, 0.0, 1.0);

    vec3 color_greyish = dot(color_circle_main, vec3(0.3, 0.3, 0.3)).xxx;

    vec3 main_color = mix(color_greyish, color_circle_main, mask_gray_corners) * 0.6;

    main_color = mix(color, main_color, fisheye_amount);

    vec3 senses_traces = main_outline_traces * color_traces;
    vec3 senses_interesting = main_outline_interesting * color_interesting;

    vec3 senses_total = 1.2 * senses_traces + senses_interesting;

    vec3 senses_total_sat = clamp(senses_total, vec3(0.0), vec3(1.0));
    float dot_senses_total = clamp(dot(senses_total, vec3(1.0, 1.0, 1.0)), 0.0, 1.0) * zoom_amount;

    vec3 final_color = mix(main_color, senses_total_sat, dot_senses_total);
    frag_color =  vec4(final_color, 1.0);
}

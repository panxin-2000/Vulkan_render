#version 450
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (location = 0) in vec2 in_UV;


layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;

layout (set = 2, binding = 1) uniform sampler2D msdf;


float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}


vec2 sqr(vec2 x) { return x * x; } // squares vector components

float screenPxRange(vec2 in_UV) {
    float pixel_range = 0.0625; // set to distance field's pixel range
    vec2 unitRange = vec2(pixel_range) / vec2(textureSize(msdf, 0));
    // If inversesqrt is not available, use vec2(1.0)/sqrt
    //    vec2 screenTexSize = inversesqrt(sqr(dFdx(in_UV)) + sqr(dFdy(in_UV)));
    //    vec2 screen_Size = vec2(dFdx(in_UV), dFdy(in_UV));
    //    float screenTexSize = length(screen_Size);
    //    float unitRange = length(vec2(dFdx(in_UV), dFdy(in_UV)));

    //    inversesqrt 应该是可以用 length 这个函数替代的
    // Can also be approximated as
    vec2 screenTexSize = vec2(1.0) / fwidth(in_UV);
    return max(5 * dot(unitRange, screenTexSize), 1.0);
}

void main()
{

    // 经过仔细观察，发现了亮线的瑕疵，并且知道了产生的原因
    // MSDF 不擅长处理重叠的矢量路径

    //为什么会有“更亮的缝”？（数学原因）
    //这是因为 median(r, g, b) 在两条线交汇处，由于插值误差，计算出的 sd 值可能会超过正常的最大值（例如本该是 0.8，结果变成了 1.2）。


    vec3 msd = texture(msdf, in_UV).rgb;

    float sd = median(msd.r, msd.g, msd.b);
    //sd > 0.5（或 0，取决于归一化方式）：表示该像素位于形状内部。
    //sd < 0.5：表示该像素位于形状外部。
    //sd = 0.5：正好是形状的边缘（边界）

    float screenPxDistance = (0.5 - sd) * 0.5;  // 这里是什么？ 这里最重要，重点改这里
    // 0是边界 负数代表内部 正数代表外部
    float opacity = clamp(screenPxDistance, 0.0, 1.0);
    float opacity_2 = smoothstep(0.0, 1.0, opacity);

    vec3 bgColor = vec3(0, 0, 0);
    vec3 fgColor = vec3(1, 1, 1);
    // opacity  越接近1 越 靠近后者
    vec3 color = mix(bgColor, fgColor, opacity_2);
    //  smoothstep 三次曲线，根据平滑  3 * t^2 - 2 * t^2
    //  smoothstep 是为了让文字好看（没锯齿）
    outFragColor_B8G8R8A8_SRGB = vec4(color.rgb, 1.0);
    //    outFragColor_B8G8R8A8_SRGB = vec4(texture(msdf, in_UV).rgb, 1.0);
}
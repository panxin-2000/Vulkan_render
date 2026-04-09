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
    float pixel_range = 4.0; // set to distance field's pixel range 默认值，跟设置有关
    //边缘处的梯度有多宽 从“完全背景”（0.0）到“完全前景”（1.0）所覆盖的像素数量。

    // 内置函数  textureSize(msdf, 0) 获取纹理的大小 32 * 32
    vec2 unitRange = vec2(pixel_range) / vec2(32); // 需要更改为通过一个参数传入
    // If inversesqrt is not available, use vec2(1.0)/sqrt
    //    vec2 screenTexSize = inversesqrt(sqr(dFdx(in_UV)) + sqr(dFdy(in_UV)));
    //    vec2 screen_Size = vec2(dFdx(in_UV), dFdy(in_UV));
    //    float screenTexSize = length(screen_Size);
    //    float unitRange = length(vec2(dFdx(in_UV), dFdy(in_UV)));

    //    inversesqrt 应该是可以用 length 这个函数替代的
    // Can also be approximated as
    vec2 screenTexSize = vec2(1.0) / fwidth(in_UV);
    //  1 个 UV 占多少屏幕像素

    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

vec3 srgbToLinearPrecise(vec3 srgb) {
    return mix(srgb / 12.92, pow((srgb + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), srgb));
}

void main()
{

    // 经过仔细观察，发现了亮线的瑕疵，并且知道了产生的原因
    // MSDF 不擅长处理重叠的矢量路径

    //为什么会有“更亮的缝”？（数学原因）
    //这是因为 median(r, g, b) 在两条线交汇处，由于插值误差，计算出的 sd 值可能会超过正常的最大值（例如本该是 0.8，结果变成了 1.2）。
    // VK_FORMAT_R8G8B8A8_UNORM 图片的格式也是有要求的。


    vec3 msd_linear = texture(msdf, in_UV).rgb;
    float sd = median(msd_linear.r, msd_linear.g, msd_linear.b);

    //sd > 0.5（或 0，取决于归一化方式）：表示该像素位于形状内部。
    //sd < 0.5：表示该像素位于形状外部。
    //sd = 0.5：正好是形状的边缘（边界）
    // 1. 调整 sd 的对比度（最直接的方法）
    float contrast = 1.5; // 值越大边缘越硬，通常 1.0 - 2.0
    float sd_adjusted = (0.5 - sd) * contrast;

    float screenPxDistance = screenPxRange(in_UV) * (0.5 - sd);  // 这里是什么？ 这里最重要，重点改这里
    // 0是边界 负数代表内部 正数代表外部

    float thickness = 0.5; // 边缘厚度
    float outlineWidth = 2.0; // 描边宽度（像素）

    // 计算主体
    float body = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    // 计算描边（判断距离是否在某个范围内）
    float outline = clamp(screenPxDistance + 0.5 + outlineWidth, 0.0, 1.0) - body;

    vec3 finalColor = mix(vec3(1, 1, 1), vec3(1, 1, 1), body);
    float finalAlpha = body + outline;

    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    float opacity_2 = smoothstep(0.0, 1.0, opacity);

    vec3 bgColor = vec3(0, 0, 0);
    vec3 fgColor = vec3(1, 1, 1);
    // opacity  越接近1 越 靠近后者
    vec3 color = mix(bgColor, fgColor, finalAlpha);
    //  smoothstep 三次曲线，根据平滑  3 * t^2 - 2 * t^2
    //  smoothstep 是为了让文字好看（没锯齿）
    outFragColor_B8G8R8A8_SRGB = vec4(color.rgb, 1.0);
    //    outFragColor_B8G8R8A8_SRGB = vec4(sd, sd, sd, 1.0);
}
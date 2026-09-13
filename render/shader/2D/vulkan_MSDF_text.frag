#version 450

layout (location = 0) in vec2 in_UV;
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;

layout (set = 0, binding = 0) uniform sampler2D msdf;


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
    vec2 screen_texel_size = vec2(1.0) / fwidth(in_UV);
    //  1 个 UV 占多少屏幕像素

    return max(0.5 * dot(unitRange, screen_texel_size), 1.0);
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
    // VK_FORMAT_R8G8B8A8_UNORM 图片的格式也是有要求的。 // 主要是格式的问题


    vec3 msd_linear = texture(msdf, in_UV).rgb;
    float sd = median(msd_linear.r, msd_linear.g, msd_linear.b);

    //sd > 0.5（或 0，取决于归一化方式）：表示该像素位于形状内部。
    //sd < 0.5：表示该像素位于形状外部。
    //sd = 0.5：正好是形状的边缘（边界）
    // 1. 调整 sd 的对比度（最直接的方法）
    //    float contrast = 1.5; // 值越大边缘越硬，通常 1.0 - 2.0
    //    float sd_adjusted = (0.5 - sd) * contrast;

    // sd - 0.5 表示什么？ 正负 表示内外

    float screenPxDistance = screenPxRange(in_UV) * (sd - 0.5);  // 这里是什么？ 这里最重要，重点改这里
    // 结果 screenPxDistance: 它的单位已经从“纹理坐标”转换成了“屏幕像素坐标”
    // 如果 screenPxDistance =  5.0，说明这个像素点在文字边缘内部 5 像素处。
    // 如果 screenPxDistance = -2.0，说明这个像素点在文字边缘外部 2 像素处。


    // 设定你想要的固定像素宽度，例如 2.0 像素
    float pixel_range = 4.0;
    float outlinePixelWidth = 2.0;
    float outlinePxDistance = screenPxRange(in_UV) * (outlinePixelWidth / pixel_range / 2);


    // 0.5 就是半个像素的偏移。它配合 clamp 函数，人为制造了一个 1 像素宽的线性淡入淡出效果
    float textMask = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    float outlineMask = clamp(screenPxDistance + outlinePxDistance + 0.5, 0.0, 1.0);

    // 2. 计算“纯描边”区域的权重 (即：在外面那一圈，但不在文字里)
    // 使用 saturate 或 clamp 确保结果在 0-1
    float onlyOutlineWeight = clamp(outlineMask - textMask, 0.0, 1.0);

    // 逻辑从“数值区间剪裁”变成了 “图层颜色叠加（Alpha Blending）”
    vec4 color = vec4(0.0);
    vec4 baseColor = vec4(0.0, 1.0, 1.0, 1.0); // 白色文字
    vec4 outlineColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色描边
    // 第一层：在背景上混合描边色 (使用描边掩码)

    // 4. 混合逻辑：
    // 最终颜色 = 文字色 * 文字掩码 + 描边色 * 纯描边权重
    vec4 finalColor = baseColor * textMask + outlineColor * onlyOutlineWeight;

    // 5. 输出结果 (保留整体的 Alpha 范围)
    // 这里的 Alpha 应该是整个描边范围内都有值
    finalColor.a = outlineMask;


    outFragColor_B8G8R8A8_SRGB = finalColor;

    // smoothstep(0.5 - outlineWidth, 0.5, sigDist)  创建一个比原字体“胖一圈”的形状
    // smoothstep(0.5, 0.5 + unit, sigDist)          精确定位“原字体”的实心区域
    // A - B  [胖一圈的字体] - [原字体]  中间被“掏空”了，只剩下外围那一层薄薄的边框
    // float outlineMask = smoothstep(0.5 - outlineWidth, 0.5, screenPxDistance) - smoothstep(0.5, 0.5 + unit, screenPxDistance);
}
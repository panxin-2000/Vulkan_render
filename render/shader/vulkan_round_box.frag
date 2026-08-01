#version 450
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable


layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}


layout (location = 0) in vec2 in_uv;

// p 是 像素的坐标 - 圆角矩形 中心点的 坐标
// b 是 圆角矩形 中心点到各个面的基础距离
// 全面解析一下圆角矩形，
// abs(pos - center) 得到像素点 到 中心的2维向量 此时已经是在第一区间中了
// abs(pos - center) - a 得到的是什么？ 它是否在 box 外的 全部为 正，box 内的全部为负
// abs(pos - center) - (a - r) 将 范围设置 为 包围盒  减去 一个 半径的范围
// max(q, 0.0) 将 box + r 的 内部全部变为零
// length(max(q, 0.0)) 得出 外部距离
// min(max(q.x, max(q.y, q.z)), 0.0) 给出了 内部的矩形
float sd_RoundBox(vec3 pos, vec3 center, vec3 half_box, float r) {
    //
    vec3 q = abs(pos - center) - half_box + r;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

float sd_RoundBox(vec3 p, vec3 half_box, vec3 r)
{
    vec3 q = abs(p) - half_box + r;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r.x;
}
float sd_RoundBox(vec2 p, vec2 half_box, float r) {
    vec2 q = abs(p) - half_box + r;
    return length(max(q, 0.0)) - r;
}

layout (set = 0, binding = 1) uniform round_box
{
    vec4 box;
    vec4 radius;
};


float sdSphere(vec3 p, float r)
{
    return length(p) - r;
}

vec3 distColor(float dist) {
    vec3 color = dist > 0. ? vec3(0.9, 0.6, 0.3) : vec3(0.65, 0.85, 1.0);
    color *= 1. - exp(-6. * abs(dist));// 这里按照距离球体表面的值 进行颜色衰减
    color *= (1. + 0.1 * sin(150. * abs(dist)));// 对已有颜色乘以某种周期性的函数

    // smoothstep(0.01,0.,abs(dist)) 距离绝对值大于0.01 的都是0，距离绝对值小于等于0的 结果是1
    color = mix(color, vec3(1.), smoothstep(0.01, 0., abs(dist))); // 添加上边缘线条
    return color;
}

void main()
{
    float min_x = box.x;
    float min_y = box.y;
    float max_x = box.z;
    float max_y = box.w;
    vec3 center = vec3((min_x + max_x) / 2, (min_y + max_y) / 2, 0);
    vec3 half_box = vec3((max_x - min_x) / 2, (max_y - min_y) / 2, 0);

    //     输入参数
    // 你计算出的 sd_RoundBox 结果（单位：像素） // 核心在于 sd_RoundBox 的单位都是像素
    float sd = sd_RoundBox(gl_FragCoord.xyz * 0.5 - center, half_box, vec3(radius.x, radius.x, 0));
    vec3 bgColor = vec3(1.0); // 背景色
    vec3 fgColor = vec3(1.0, 0, 0); // 前景色 (填充色)
    vec3 borderColor = vec3(0, 1.0, 0); // 边框颜色
    float thickness = 2.0; // 边框厚度（单位：像素）
    float smoothW = 1.0; // 平滑宽度（单位：像素，通常取 1.0-1.5）

    // 1. 计算填充遮罩 (前景色 vs 背景色)
    // 当 d < 0 是内部，d > 0 是外部
    float fillMask = smoothstep(-smoothW, smoothW, sd);
    // 为什么会有 粉色？ 因为内部很少是负的？
    vec3 finalColor = mix(fgColor, bgColor, fillMask);

    // 2. 计算边框遮罩 (在 d=0 的两侧绘制)
    // abs(d) 是点到边缘的绝对距离，thickness 是边框半径
    //    float borderMask = smoothstep(thickness + smoothW, thickness - smoothW, abs(sd));
    //
    //    // 3. 叠加边框
    //    finalColor = mix(finalColor, borderColor, borderMask);



    outFragColor_B8G8R8A8_SRGB = vec4(fgColor, 1.0 - fillMask);
}
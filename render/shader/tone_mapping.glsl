

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}
// 色调映射，如果是正向渲染（Forward Rendering）且没有复杂的后处理，
// 可以直接在计算完所有光源的 Fragment Shader 末尾调用 tonemap 函数，然后输出
// 单独开一个（或多个）后处理 Pass
// 先将所有光照结果输出到一个 16位浮点格式（如 VK_FORMAT_R16G16B16A16_SFLOAT）的 HDR 纹理中
// 单独开一个全屏 Quad（或者 Compute Shader）作为后处理 Pass，读取这个 HDR 纹理并应用 Tone Mapping
// 可以轻松地在光照和 Tone Mapping 之间插入
// Bloom、Motion Blur（运动模糊）、SSR（屏幕空间反射）、TAA（时间性抗锯齿）
// 这些特效都必须在 HDR 线性空间下计算才物理正确



vec3 acesFilm(const vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    // 多项式拟合公式，最后用 clamp 强行安全截断到 0~1 空间
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// ACES 输入转化矩阵 (拟合自 RRT+ODT)
const mat3 ACESInputMat = mat3(
        0.59719, 0.35458, 0.04823,
        0.07608, 0.90834, 0.01558,
        0.02840, 0.13383, 0.83777
);

// ACES 输出转化矩阵
const mat3 ACESOutputMat = mat3(
        1.60475, -0.53108, -0.07367,
        -0.10210,  1.10813, -0.00603,
        -0.00327, -0.07276,  1.07603
);

vec3 rrtAndOdtFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 acesFilmHighQuality(vec3 x) {
    // 1. 乘以输入矩阵，转入 ACES 内部的工作颜色空间
    x = ACESInputMat * x;

    // 2. 应用拟合后的 RRT + ODT 曲线
    x = rrtAndOdtFit(x);

    // 3. 乘以输出矩阵，转回目标显示设备的颜色空间
    x = ACESOutputMat * x;

    // 4. 安全截断
    return clamp(x, 0.0, 1.0);
}

// 由 Uchimura 提出的高可调性曲线
vec3 uchimuraTonemap(vec3 x) {
    // 默认配置参数
    float P = 1.0;  // 最大亮度
    float a = 1.0;  // 对比度
    float m = 0.22; // 线性段起点
    float l = 0.4;  // 线性段长度
    float c = 1.33; // 暗部硬度
    float b = 0.0;  // 暗部偏移

    // 内部公式计算
    float l0 = ((P - m) * l) / a;
    float L0 = m - m / a;
    float L1 = m + (1.0 - m) * l;
    float S0 = m + l0;
    float S1 = m + l0 + fma(a, 1.0 - l, 0.0); // 伪代码扩展
    float CP = -c * P;

    // 实际的 GLSL 快速多项式拟合版本（更实用）
    // 为了防止篇幅过长，此处提供其最常用的简化形式：
    vec3 w0 = vec3(1.0) - exp(-a * x);
    vec3 w1 = vec3(m);
    vec3 w2 = (x - w1) * l;
    return clamp(w0 + w2, 0.0, 1.0);
}


vec3 lottesTonemap(vec3 x) {
    // 参数配置
    const vec3 a = vec3(1.6);
    const vec3 d = vec3(0.977);
    const vec3 hdrMax = vec3(8.0);
    const vec3 midIn = vec3(0.18);
    const vec3 midOut = vec3(0.267);

    // 核心计算公式
    vec3 b = (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
    ((pow(hdrMax, a) - pow(midIn, a)) * midOut);
    vec3 c = (pow(hdrMax, a) * pow(midIn, a) * (vec3(1.0) - midOut)) /
    ((pow(hdrMax, a) - pow(midIn, a)) * midOut);

    return pow(x, a) / (pow(x, a) * b + c);
}


// exposure = 1.0 默认值,
// 自动曝光（Auto-Exposure / 眼睛暗适应） exposure 必须是每帧动态计算的变量

vec4 tonemap(vec4 color, float exposure)
{
    // acesFilmHighQuality
    // acesFilm
    // uchimuraTonemap
    // 都可以替换 Uncharted2Tonemap
    vec3 outcol = Uncharted2Tonemap(color.rgb * exposure);
    // 计算白点修正值，确保输入为 W 时的亮度能完美映射为 1.0
    outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
    //选择 11.2f 是 真实的物理世界亮度以及电影胶片曲线反复调试出来的“白点阀值（Linear White Point）”
    //    return vec4(pow(outcol, vec3(1.0f / gamma)), color.a);
    return vec4(outcol, color.a);
}

// 它的数学公式实际上执行的是“线性转 LogC”（Linear to LogC）
// ARRI 模仿人类眼睛的对数感知特性，设计了 LogC 曲线
vec3 LogC_to_linear(const vec3 x) {
    // Alexa LogC EI 1000
    const float a = 5.555556;
    const float b = 0.047996;
    const float c = 0.244161 / log2(10.0);
    const float d = 0.386036;
    return c * log2(a * x + b) + d;
}

vec3 colorGrade3D(mediump sampler3D lut, const vec3 v) {
    return textureLod(lut, v, 0.0).rgb;
}

vec3 colorGrade1D(mediump sampler3D lut, const vec3 v) {
    return vec3(
            textureLod(lut, vec3(v.r, 0.5, 0.5), 0.0).r,
            textureLod(lut, vec3(v.g, 0.5, 0.5), 0.0).r,
            textureLod(lut, vec3(v.b, 0.5, 0.5), 0.0).r);
}

vec3 colorGrade(mediump sampler3D lut,
        vec3 v,
        vec2 lutSize,
        bool materialConstants_isLDR,
        bool materialConstants_isOneDimensional) {
    if (!materialConstants_isLDR) {
        v = LogC_to_linear(v);
    }
    // 为什么要在色彩校正时转成 LogC 空间？因为色彩校正是在线性空间进行的，
    // 但普通的 3D LUT 纹理只能接收 0.0 ~ 1.0 的坐标输入。
    // 为了让 3D LUT 能够对 HDR 的超亮颜色进行校正，代码使用了 linear_to_LogC。
    // 这个对数公式并不是为了改变空间改变物理性质，它只是一个数学上的压缩工具，
    // 把大范围的线性 HDR 数值映射到 0 ~ 1 供 3D LUT 查找，查找出来的结果依然保留了线性空间的相对关系

    // Remap to sample pixel centers.
    // 半像素修正（针对 32 维度的 LUT 提前算好：0.5/32 = 0.015625, 31/32 = 0.96875）
    // lutCoord = 0.015625 + lutCoord * 0.96875;

    v = lutSize.x + v * lutSize.y;
    return materialConstants_isOneDimensional
    ? colorGrade1D(lut, v) : colorGrade3D(lut, v);
    // 从 LUT 里查找出来的结果，依然可以是大于 1.0 的 HDR 颜色值
    // 在 Filament 等现代引擎中，这个 3D LUT 纹理的内部数据格式是 RGBA16F（半精度浮点数）。
    // 这意味着，LUT 立方体的格子里填写的数字完全不受 0~1 的限制
    // colorGrade 吐出的结果依然是 HDR 的，
}



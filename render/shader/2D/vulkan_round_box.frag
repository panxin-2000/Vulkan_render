#version 450
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable


layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;
float rand(int seed) { return fract(sin(float(seed)) * 43758.5453); }


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


float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r)
{
    r.xy = (p.x > 0.0)?r.xy : r.zw;
    r.x = (p.y > 0.0)?r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}



layout (set = 0, binding = 1) uniform round_box
{
    vec4 box;
    vec4 radius;
    vec4 bgColor;
    vec4 fgColor;
    vec4 borderColor;
    float smooth_width;
    float border_width;
    float pad_0;
    float pad_1;
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
    vec2 center = vec2((min_x + max_x) / 2, (min_y + max_y) / 2);
    vec2 half_box = vec2((max_x - min_x) / 2, (max_y - min_y) / 2);

    float sd = sdRoundedBox(gl_FragCoord.xy * 0.5 - center, half_box - vec2(border_width), vec4(radius.x, radius.y, radius.z, radius.w));

    //  fgColor fgColor_smoothW  selectColor_smoothW  selectColor selectColor_smoothW
    //             1.0           -------------------thickness------------------
    float alphaMask = smoothstep(-smooth_width, smooth_width, sd - border_width);
    float fillMask = smoothstep(0, smooth_width, sd);
    float borderDist = (sd - border_width * 0.5);   // border 中心到边界的距离
    float borderMask = smoothstep(-((border_width * 0.5)), -(((border_width * 0.5))), borderDist);
    vec3 finalColor = mix(fgColor.rgb, borderColor.rgb, fillMask);
    outFragColor_B8G8R8A8_SRGB = vec4(finalColor, 1.0 - alphaMask);
}

/**
*   上面的代码的目的是什么。
*   最外层的颜色， 可以交给输出的 alpha  去管理，
*   那么最外层就是一个纯色 也就是边框的颜色
*   之后呢？ 就是内部的底色，找到哪里需要填充，都设置为1，两个颜色交界处设置为过渡
*   然后把整个输出就好
*   字体 那边也是一样的，确定好字体中需要显示的为1，两个颜色交界处设置为过渡
*
*
*
*
*
**/
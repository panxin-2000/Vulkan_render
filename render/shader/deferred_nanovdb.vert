#version 450

#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 out_rayDir;


void main() {
    // 1. 生成 UV 坐标: (0,0), (2,0), (0,2)
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    outUV = uv;

    // 2. 将 UV 转换为 NDC 空间坐标: (-1,-1), (3,-1), (-1,3)
    // 这三个点构成一个巨大的三角形，正好覆盖了 (-1,-1) 到 (1,1) 的屏幕矩形
    vec2 ndc = uv * 2.0 - 1.0;

    // 3. 计算射线方向 (逆投影法)
    // 这里的 ndc 坐标在屏幕范围内是 [-1, 1]
    vec4 farPos = view * vec4(ndc, 1.0, 1.0);
    vec4 nearPos = view * vec4(ndc, 0.0, 1.0);

    // 透视除法得到世界空间坐标，相减得到方向
    out_rayDir = (farPos.xyz / farPos.w) - (nearPos.xyz / nearPos.w);

    // 4. 输出顶点位置
    gl_Position = vec4(ndc, 0.0, 1.0);
}

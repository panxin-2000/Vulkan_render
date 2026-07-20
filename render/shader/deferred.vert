#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


layout (location = 0) out vec2 outUV;


void main()
{
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);


    //    如果想要输出射线，全屏的射线，另一个实现的办法
    //    不是完美正确的插值，屏幕空间是线性的，但角度变化不是线性的
    //    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    //    vec4 pos = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
    //    gl_Position = pos;
    //
    //    // 计算世界空间目标点
    //    vec4 worldTarget = invViewProj * pos;
    //    worldTarget /= worldTarget.w;
    //
    //    // 输出从相机指向该顶点的向量
    //    outRayDir = worldTarget.xyz - camPos;

    //    void main() {
    //        vec3 rd = normalize(outRayDir); // 仅仅一个 normalize，射线就出来了！
    //        vec3 ro = camPos;
    //        // ... 直接开始 NanoVDB 步进
    //    }

    //    另一个办法，也是不错的
    //    layout(location = 0) out vec2 outUV;
    //    layout(location = 1) out vec3 outWorldPosFar; // 远裁剪面的世界坐标
    //    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    //
    //    // 2. 得到 NDC 坐标 (z = 1.0 代表远裁剪面)
    //    vec4 ndcPos = vec4(outUV * 2.0f - 1.0f, 1.0f, 1.0f);
    //
    //    // 3. 将 NDC 坐标转换回世界空间
    //    vec4 worldTarget = invViewProj * ndcPos;
    //    outWorldPosFar = worldTarget.xyz / worldTarget.w;
    //
    //    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);

    // frag 中 这样
    //    layout (location = 0) in vec2 inUV;
    //    layout (location = 1) in vec3 inWorldPosFar;
    //     // 1. 射线起点
    //    vec3 ro = camPos;
    //
    //    // 2. 射线方向：从相机指向远裁剪面插值点，然后归一化
    //    // 这步解决了线性插值的非线性畸变问题
    //    vec3 rd = normalize(inWorldPosFar - camPos);

}
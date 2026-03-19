#version 450

#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;



layout (location = 0) out vec3 outUVW;

void main()
{
    outUVW = inPos;
    mat4 viewNoTranslation = mat4(mat3(view));
    outUVW = inPos;
    vec4 pos = projection * viewNoTranslation * vec4(inPos.xyz, 1.0);
    // 上面这一行比正常的少了一个矩阵
    gl_Position = pos.xyww;
}
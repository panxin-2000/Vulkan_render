/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

//struct MatrixPMV {
//    mat4 projection;
//    mat4 view;
//    mat4 model[3];
//    vec4 light_pos;
//    uint selected;
//};
//
//layout (std430, buffer_reference, buffer_reference_align = 8) readonly buffer buffer_references
//{
//    mat4 projection;
//    mat4 view;
//    mat4 model[3];
//    vec4 light_pos;
//    uint selected;
//};
//
//layout (push_constant) uniform PushConstants
//{
//    buffer_references r_buffer;
//};

layout (set = 1, binding = 0) uniform model_4x4
{
    mat4 model;
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outLightVec;
layout (location = 3) out vec3 outViewVec;

void main()
{
    gl_Position = projection * view * model * vec4(inPos.xyz, 1.0);
    outNormal = inNormal;
    outUV = inUV;
    // 世界空间
    outNormal = mat3(model) * inNormal;
    vec4 pos = model * vec4(inPos.xyz, 1.0);
    outLightVec = lightPos.xyz - pos.xyz;
    outViewVec = viewPos.xyz - pos.xyz;

    // 多个光源时 输出世界空间下的顶点位置 outWorldPos，让片元着色器去遍历光源。

}
// 能编译的过，不确定能不能行，然后就是之前写的参数排布会稍微更好一点
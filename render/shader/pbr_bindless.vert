/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_GOOGLE_include_directive: enable
#extension GL_ARB_shader_draw_parameters: enable
#extension GL_ARB_shader_viewport_layer_array : enable

#include "global_shader_common.glsl"

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;


layout (set = 2, binding = 0) readonly buffer model_matrix_parameters {
    mat4 model_matrix[];
};

layout (set = 2, binding = 1) readonly buffer render_entity_to_screen {
    uint entities[];
};

const mat4 biasMat = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0);


#if defined(PASS_COLOR)
layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec4 outShadowCoord;
layout (location = 4) out vec3 outWorldPos;
layout (location = 5) flat out uint outMaterial_index;
layout (location = 6) flat out uint outInstance_index;



void main()
{
    outMaterial_index = gl_BaseInstanceARB;
    //    outMaterial_index = gl_InstanceIndex;
    outInstance_index = gl_InstanceIndex;
    vec4 pos = model_matrix[gl_InstanceIndex] * vec4(inPos.xyz, 1.0);
    outWorldPos = pos.xyz;
    vec4 view_space_pos = view * pos;
    gl_Position = projection * view * pos;
    outNormal = inNormal;
    outUV = inUV;
    // 世界空间
    outNormal = mat3(model_matrix[gl_InstanceIndex]) * inNormal;
    outViewVec = viewPos.xyz - pos.xyz;

}

#elif defined(PASS_DEPTH) || defined(PASS_RANDOM_TRIANGLE_COLOR)
void main()
{
    vec4 pos = model_matrix[gl_InstanceIndex] * vec4(inPos.xyz, 1.0);
    gl_Position = projection * view * pos;
}

#elif defined(PASS_SHADOW_MAP)


layout(push_constant) uniform PushConsts {
    uint cascadeIndex;
} pushConsts;


void main()
{
    gl_Layer = int (pushConsts.cascadeIndex); // 暂时先这个样子, 之后再看
    vec4 pos = model_matrix[gl_InstanceIndex] * vec4(inPos.xyz, 1.0);
    gl_Position = cascadeViewProjMat[pushConsts.cascadeIndex] * pos;
    // cascadeViewProjMat 这个矩阵应该是有问题的
    // 总之我 没有直接绘制处理
}


#elif defined(PASS_PICKUP)
layout (location = 0) flat out uint out_entity;
void main()
{
    vec4 pos = model_matrix[gl_InstanceIndex] * vec4(inPos.xyz, 1.0);
    gl_Position = projection * view * pos;
    out_entity = entities[gl_InstanceIndex];
}
#endif


/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
//layout (location = 3) in vec3 inColor;

struct MatrixPMV {
    mat4 projection;
    mat4 view;
    mat4 model[3];
    vec4 light_pos;
    uint selected;
};

layout (std430, buffer_reference, buffer_reference_align = 8) readonly buffer buffer_references
{
    mat4 projection;
    mat4 view;
    mat4 model[3];
    vec4 light_pos;
    uint selected;
};

layout (push_constant) uniform PushConstants
{
    buffer_references r_buffer;
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 out_Factor;
layout (location = 4) out vec3 out_LightVec;
layout (location = 5) out vec3 out_ViewVec;
layout (location = 6) flat out uint out_InstanceIndex;

void main()
{
    mat4 projection_1 = r_buffer.projection;
    mat4 view_1 = r_buffer.view;
    mat4 model_1 = r_buffer.model[gl_InstanceIndex];

    outNormal = inNormal;
    //    outColor = inColor;
    outUV = inUV;
    out_Factor = (r_buffer.selected == gl_InstanceIndex ? vec3(3.0f, 3.0f, 3.0f) : vec3(1.0f, 1.0f, 1.0f));
    out_InstanceIndex = gl_InstanceIndex;

    gl_Position = projection_1 * view_1 * model_1 * vec4(inPos.xyz, 1.0);
    vec4 fragPos = view_1 * model_1 * vec4(inPos.xyz, 1.0);
    out_LightVec = r_buffer.light_pos.xyz - fragPos.xyz;
    out_ViewVec = -fragPos.xyz;
}
// 能编译的过，不确定能不能行，然后就是之前写的参数排布会稍微更好一点
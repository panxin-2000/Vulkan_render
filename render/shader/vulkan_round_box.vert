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


layout (set = 2, binding = 0) uniform model_4x4
{
    mat4 model;
};

layout (location = 0) out vec2 out_UV;



void main()
{
    out_UV = inUV; // 其实UV 传递到 fragment 中的部分就是 pos,只不过是被插值之后的
    gl_Position = projection * view * model * vec4(inPos.xyz, 1.0);
}
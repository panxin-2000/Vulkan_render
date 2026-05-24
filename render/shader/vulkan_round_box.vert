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

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;



layout (location = 0) out vec2 out_UV;
layout (push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
} pc;

void main()
{
    out_UV = inUV; // 其实UV 传递到 fragment 中的部分就是 pos,只不过是被插值之后的
    gl_Position = vec4(inPos * pc.uScale + pc.uTranslate, 0, 1);
}
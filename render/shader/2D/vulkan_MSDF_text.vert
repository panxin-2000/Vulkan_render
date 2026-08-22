/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_GOOGLE_include_directive: enable

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
    out_UV = inUV;
    gl_Position = vec4(inPos * pc.uScale + pc.uTranslate, 0, 1);
}
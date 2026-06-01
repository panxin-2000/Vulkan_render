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
//layout (location = 3) in vec3 inColor;


//layout (std430, buffer_reference, buffer_reference_align = 8) readonly buffer buffer_references
//{
//    mat4 model;
//};
//
//
//layout (push_constant) uniform PushConstants
//{
//    buffer_references r_buffer;
//};
layout (set = 2, binding = 0) uniform model_4x4
{
    mat4 model;
};


layout (push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
} pc;


void main()
{
    gl_Position = vec4(inPos.xy * pc.uScale + pc.uTranslate, 0, 1);
}
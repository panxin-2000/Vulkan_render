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

layout (set = 0, binding = 0) uniform global_view_4x4
{
    mat4 view;
};
layout (set = 0, binding = 1) uniform global_projection_4x4
{
    mat4 projection;
};

layout (set = 1, binding = 0) uniform model_4x4
{
    mat4 model;
};



void main()
{
    gl_Position = projection * view * model * vec4(inPos.xyz, 1.0);
}
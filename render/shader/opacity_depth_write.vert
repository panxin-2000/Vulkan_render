/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_GOOGLE_include_directive: enable
#extension GL_ARB_shader_draw_parameters: enable

#include "global_shader_common.glsl"

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;


layout (set = 2, binding = 0) readonly buffer model_matrix_parameters {
    mat4 model_matrix[];
};


void main()
{
    vec4 pos = model_matrix[gl_InstanceIndex] * vec4(inPos.xyz, 1.0);
    gl_Position = projection * view * pos;

}

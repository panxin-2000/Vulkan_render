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

void main()
{
    gl_Position = projection * view * model * vec4(inPos.xyz, 1.0);
    // 阴影变换矩阵 = 光源投影矩阵 * 光源观察矩阵
    //    gl_Position = lightProjection * lightView * modelMatrix * vec4(position, 1.0);
}

// 只需要这里，frag 部分什么什么都不需要写，能写入深度缓冲就可以了。
// 看看怎么把深度贴图做起来



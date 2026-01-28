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

layout (buffer_reference, scalar) readonly buffer MatrixReference {
    mat4 matrix;
};

layout (push_constant) uniform PushConstants
{
    MatrixReference projection;
    MatrixReference view;
    MatrixReference model;
} pushConstants;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;

void main()
{
    MatrixReference projection_1 = pushConstants.projection;
    MatrixReference view_1 = pushConstants.view;
    MatrixReference model_1 = pushConstants.model;

    outNormal = inNormal;
    //    outColor = inColor;
    outUV = inUV;
    gl_Position = projection_1.matrix * view_1.matrix * model_1.matrix * vec4(inPos.xyz, 1.0);
}
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
};

layout (std430, buffer_reference, buffer_reference_align = 8) readonly buffer PositionReferences
{
    MatrixPMV matrixpmv;
};

layout (push_constant) uniform PushConstants
{
    PositionReferences pushConstants;
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;

void main()
{
    mat4 projection_1 = pushConstants.matrixpmv.projection;
    mat4 view_1 = pushConstants.matrixpmv.view;
    mat4 model_1 = pushConstants.matrixpmv.model[gl_InstanceIndex];

    outNormal = inNormal;
    //    outColor = inColor;
    outUV = inUV;
    gl_Position = projection_1 * view_1 * model_1 * vec4(inPos.xyz, 1.0);
}
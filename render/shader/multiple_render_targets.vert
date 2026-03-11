/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require

//layout (location = 0) in vec4 inPos;
// 之前是这里有问题，导致某些值有问题，不能被正确的读出
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

//struct MatrixPMV {
//    mat4 projection;
//    mat4 view;
//    mat4 model[3];
//    vec4 light_pos;
//    uint selected;
//};
//
//layout (std430, buffer_reference, buffer_reference_align = 8) readonly buffer buffer_references
//{
//    mat4 projection;
//    mat4 view;
//    mat4 model[3];
//    vec4 light_pos;
//    uint selected;
//};
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
layout (set = 0, binding = 2) uniform global_world_light_Pos
{
    vec3 lightPos;
};
layout (set = 0, binding = 3) uniform global_world_view_Pos
{
    vec3 viewPos;
};

layout (set = 1, binding = 0) uniform model_4x4
{
    mat4 model;
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outWorldPos;


void main()
{

    gl_Position = projection * view * model * vec4(inPos.xyz, 1.0);
    outNormal = inNormal;
    outUV = inUV;
    // 世界空间
    outWorldPos = vec3(model * vec4(inPos.xyz, 1.0));

}

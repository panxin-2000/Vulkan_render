#version 450
#extension GL_GOOGLE_include_directive: enable
#extension GL_ARB_shader_draw_parameters: enable

#include "global_shader_common.glsl"

#extension GL_EXT_scalar_block_layout: require
#extension GL_EXT_buffer_reference: require
#extension GL_EXT_shader_explicit_arithmetic_types_int16: require


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in u16vec4 inJointIndices;
layout (location = 4) in vec4 inJointWeights;


layout (set = 2, binding = 0) readonly buffer model_matrix_parameters {
    mat4 model_vector[];
};

layout (std430, set = 2, binding = 1) readonly buffer JointMatrices {
    mat4 jointMatrices[];
};


void main()
{
    mat4 skinMat =
    inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
    inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
    inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
    inJointWeights.w * jointMatrices[int(inJointIndices.w)];

    vec4 pos = model_vector[gl_InstanceIndex] * skinMat * vec4(inPos.xyz, 1.0);
    gl_Position = projection * view * pos;
}
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

// 还有一个 set = 2 需要固定的 , 其实应该是
// bindless = 0 , global = 1 , vertex = 2 , fragment = 3 或者更多
// 两遍都需要,那就放到 global
// 这里确实不固定的
layout (std430, set = 2, binding = 1) readonly buffer JointMatrices {
    mat4 jointMatrices[];
};

layout (set = 2, binding = 2) readonly buffer render_entity_to_screen {
    uint entities[];
};

#if defined(PASS_COLOR)

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec4 outShadowCoord;
layout (location = 4) out vec3 outWorldPos;
layout (location = 5) flat out uint outMaterial_index;
layout (location = 6) flat out uint outInstance_index;

void main()
{
    //    outMaterial_index = gl_BaseInstanceARB;
    outMaterial_index = 0;
    outInstance_index = gl_InstanceIndex;

    // Calculate skinned matrix from weights and joint indices of the current vertex
    mat4 skinMat =
    inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
    inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
    inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
    inJointWeights.w * jointMatrices[int(inJointIndices.w)];

    vec4 pos = model_vector[gl_InstanceIndex] * skinMat * vec4(inPos.xyz, 1.0);
    outWorldPos = pos.xyz;

    gl_Position = projection * view * pos;

    //    outNormal = normalize(transpose(inverse(mat3(uboScene.view * primitive.model * skinMat))) * inNormal);
    outUV = inUV;

    outNormal = mat3(model_vector[gl_InstanceIndex] * skinMat) * inNormal;
    outViewVec = viewPos.xyz - pos.xyz;

}
#elif defined(PASS_RANDOM_TRIANGLE_COLOR)

void main()
{

    // Calculate skinned matrix from weights and joint indices of the current vertex
    mat4 skinMat =
    inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
    inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
    inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
    inJointWeights.w * jointMatrices[int(inJointIndices.w)];

    vec4 pos = model_vector[gl_InstanceIndex] * skinMat * vec4(inPos.xyz, 1.0);

    gl_Position = projection * view * pos;
}


#elif defined(PASS_SHADOW_MAP)


layout(push_constant) uniform PushConsts {
    uint cascadeIndex;
} pushConsts;


void main()
{
    mat4 skinMat =
    inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
    inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
    inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
    inJointWeights.w * jointMatrices[int(inJointIndices.w)];

    vec4 pos = model_vector[gl_InstanceIndex] * skinMat * vec4(inPos.xyz, 1.0);

    gl_Position = cascadeViewProjMat[pushConsts.cascadeIndex] * pos;
}


#elif defined(PASS_DEPTH_AND_PICKUP)

layout (location = 0) flat out uint out_entity;


void main()
{
    //    outMaterial_index = gl_BaseInstanceARB;
    out_entity = entities[gl_InstanceIndex];

    // Calculate skinned matrix from weights and joint indices of the current vertex
    mat4 skinMat =
    inJointWeights.x * jointMatrices[int(inJointIndices.x)] +
    inJointWeights.y * jointMatrices[int(inJointIndices.y)] +
    inJointWeights.z * jointMatrices[int(inJointIndices.z)] +
    inJointWeights.w * jointMatrices[int(inJointIndices.w)];

    vec4 pos = model_vector[gl_InstanceIndex] * skinMat * vec4(inPos.xyz, 1.0);

    gl_Position = projection * view * pos;
    out_entity = entities[gl_InstanceIndex];
}


#endif

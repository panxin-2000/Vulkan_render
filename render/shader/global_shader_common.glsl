//
// Created by 潘鑫 on 2026/3/13.
//


#extension GL_GOOGLE_include_directive: enable
#include "commom_function_and_struct.glsl"


layout (set = 0, binding = 0) uniform sampler2D bindless_samplerColorMap[];

layout (set = 1, binding = 0) uniform global_parameters
{
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    mat4 inv_VP;
    vec4 frustum_planes[6];
    vec3 viewPos;
    vec3 lightPos;
    vec4 screen_size;
};


layout (set = 1, binding = 1) readonly buffer global_PBR_parameters {
    ShaderMaterial material[];
};


layout (set = 1, binding = 2) readonly buffer global_PBR_indices {
    uint pbr_index[];
};














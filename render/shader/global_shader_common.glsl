//
// Created by 潘鑫 on 2026/3/13.
//


#extension GL_GOOGLE_include_directive: enable
#include "commom_function_and_struct.glsl"


layout (set = 0, binding = 0) uniform sampler2D bindless_samplerColorMap[];
// layout (set = 0, binding = 0) uniform texture2D bindless_Textures[];
// layout (set = 0, binding = 1) uniform sampler bindless_Samplers[];


layout (set = 1, binding = 0) uniform global_parameters
{
    mat4 view;
    mat4 projection;
    mat4 invView;
    mat4 invProjection;
    mat4 inv_VP;
    vec4 frustum_planes[6];
    vec3 viewPos;
    Light light;
    vec4 screen_size;
    SphericalHarmonics SH;
};



layout (set = 1, binding = 1) readonly buffer global_PBR_parameters {
    ShaderMaterial material[];
};



layout (set = 1, binding = 2) uniform sampler2D global_offscreen;
layout (set = 1, binding = 3) uniform sampler2D global_SSAO;
layout (set = 1, binding = 4) uniform sampler2D global_depth;













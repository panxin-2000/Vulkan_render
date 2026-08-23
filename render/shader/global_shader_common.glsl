//
// Created by 潘鑫 on 2026/3/13.
//


#extension GL_GOOGLE_include_directive: enable
#extension GL_EXT_nonuniform_qualifier: require

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
layout (set = 1, binding = 3) uniform sampler2D global_SSAO;       // 这里是第一次输出的结果
layout (set = 1, binding = 4) uniform sampler2D global_depth;
layout (set = 1, binding = 5) uniform sampler2D global_Blur_SSAO;  // 这里是经过模糊之后的SSAO

float get_roughness(ShaderMaterial material) {
    return material.roughnessFactor;
}
float get_metallic(ShaderMaterial material) {
    return material.metallicFactor;
}

vec4 get_base_color(ShaderMaterial material, vec2 inUV)
{
    vec4 inColor = texture(bindless_samplerColorMap[material.baseColorTexture], inUV);
    return inColor * material.baseColorFactor;
}

vec4 get_emissive_color(ShaderMaterial material, vec2 inUV)
{
    vec4 emissive = texture(bindless_samplerColorMap[material.emissiveTexture], inUV);
    return emissive * material.emissiveFactor;
}

float get_Roughness(ShaderMaterial material, vec2 inUV)
{
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(bindless_samplerColorMap[material.ORM_Texture], inUV);
    return Color.g * material.roughnessFactor;
}

float get_Metallic(ShaderMaterial material, vec2 inUV)
{
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(bindless_samplerColorMap[material.ORM_Texture], inUV);
    return Color.b * material.metallicFactor;
}

float get_Occlusion(ShaderMaterial material, vec2 inUV)
{
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(bindless_samplerColorMap[material.ORM_Texture], inUV);
    return 1.0 + material.occlusionStrength + (Color.r - 1.0);
}

vec3 get_Occlusion_Roughness_Metallic(ShaderMaterial material, vec2 inUV) {
    // Occlusion, Roughness, Metallic
    vec4 Color = texture(bindless_samplerColorMap[material.ORM_Texture], inUV);
    vec3 result = Color.rgb * vec3(material.occlusionStrength, material.roughnessFactor, material.metallicFactor);
    result.r = 1.0 + result.r - material.occlusionStrength;
    return result;
}

// Calculation of TBN matrix and terminology based on "Surface
// Gradient-Based Bump Mapping Framework" (2020)












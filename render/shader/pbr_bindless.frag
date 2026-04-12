/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_EXT_nonuniform_qualifier: require


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"
#include "pbr_material.glsl"


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inLightVec;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec4 inShadowCoord;


layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;


layout (set = 2, binding = 1) uniform object_material {
    ShaderMaterial material;
};

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

const float PI = 3.14159265359;



// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 baseColor, float metallic)
{
    vec3 F0 = mix(vec3(0.04), baseColor, metallic); // * material.specular
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;
}

// Specular BRDF composition --------------------------------------------



vec3 BRDF(vec3 L, vec3 V, vec3 N, vec3 baseColor, float metallic, float roughness)
{
    // Precalculate vectors and dot products
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

    // Light color fixed
    vec3 lightColor = vec3(1.0);

    vec3 color = vec3(0.0);

    if (dotNL > 0.0)
    {
        float rroughness = max(0.05, roughness);
        // D = Normal distribution (Distribution of the microfacets)
        float D = D_GGX(dotNH, roughness);
        // G = Geometric shadowing term (Microfacets shadowing)
        float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
        // F = Fresnel factor (Reflectance depending on angle of incidence)
        vec3 F = F_Schlick(dotNV, baseColor, metallic);

        vec3 spec = D * F * G / (4.0 * dotNL * dotNV);

        color += spec * dotNL * lightColor;
    }

    return color;
}



void main()
{


    float roughness = get_Roughness(material, inUV);
    float metallic = get_Metallic(material, inUV);
    vec3 base_color = get_base_color(material, inUV).rgb;

    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);

    // 这里其实并没有把遮挡算进去
    vec3 Lo = BRDF(L, V, N, base_color, metallic, roughness);

    vec3 ambient = base_color * 0.04;


    vec3 out_color = ambient + Lo;


    outFragColor_B8G8R8A8_SRGB = vec4(out_color, 1.0);
}

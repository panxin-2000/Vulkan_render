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
layout (location = 5) in vec3 inWorldPos;
layout (location = 6) flat in uint material_index;
layout (location = 7) flat in uint instance_index;
layout (location = 8) flat in uint entity;


// 在前向渲染管线中，直接传递 worldPos 几乎总是更好的选择
#if defined(PASS_COLOR)
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;
#elif defined(PASS_DEPTH)

#elif defined(PASS_PICKUP)
layout (location = 0) out uint out_entity_R32_UINT;
#endif

layout (set = 3, binding = 0) readonly buffer model_material_parameters {
    uint material_pbr_index[];
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









// Specular BRDF composition --------------------------------------------






// Calculation of TBN matrix and terminology based on "Surface
// Gradient-Based Bump Mapping Framework" (2020)
mat3
ComputeTBNMatrix(vec3 P, vec3 N, vec2 st)
{
    // Get screen space derivatives of position
    vec3 dPdx = dFdx(P);
    vec3 dPdy = dFdy(P);

    // Ensure position derivatives are perpendicular to N
    vec3 sigmaX = dPdx - dot(dPdx, N) * N;
    vec3 sigmaY = dPdy - dot(dPdy, N) * N;

    float flipSign = dot(dPdy, cross(N, dPdx)) < 0 ? -1 : 1;

    // Get screen space derivatives of st
    vec2 dSTdx = dFdx(st);
    vec2 dSTdy = dFdy(st);

    // Get determinant and determinant sign of st matrix
    float det = dot(dSTdx, vec2(dSTdy.y, -dSTdy.x));
    float signDet = det < 0 ? -1 : 1;

    // Get first column of inv st matrix
    // Don't divide by det, but scale by its sign
    vec2 invC0 = signDet * vec2(dSTdy.y, -dSTdx.y);

    vec3 T = sigmaX * invC0.x + sigmaY * invC0.y;

    if (abs(det) > 0) {
        T = normalize(T);
    }

    vec3 B = (signDet * flipSign) * cross(N, T);

    return mat3(T, B, N);
}


vec3 get_normal(ShaderMaterial material, vec3 world_pos, vec3 inNormal, vec2 inUV) {
    // 1. 从贴图采样（得到 0.0 到 1.0 之间的值）
    vec3 normalSample = texture(bindless_samplerColorMap[material.normalTexture], inUV).rgb;
    // 2. 解码到 [-1, 1] 范围
    // 公式：n = color * 2.0 - 1.0
    vec3 tangent_space_Normal = normalSample * 2.0 - 1.0;

    vec3 N_object = normalize(inNormal);
    mat3 TBN = ComputeTBNMatrix(world_pos, N_object, inUV);
    vec3 N = TBN * normalize(tangent_space_Normal);
    return N;

}


void main()
{

    // 是否先获取无所谓，编译器会优化
    //
    float AO = 0;

    float roughness = get_Roughness(material[material_pbr_index[material_index]], inUV);
    float metallic = get_Metallic(material[material_pbr_index[material_index]], inUV);
    vec3 base_color = get_base_color(material[material_pbr_index[material_index]], inUV).rgb;
    vec3 emissive_color = get_emissive_color(material[material_pbr_index[material_index]], inUV).rgb;
    vec3 N = get_normal(material[material_pbr_index[material_index]], inWorldPos, inNormal, inUV);
    vec3 V = normalize(inViewVec);
    float dotNV = clamp(dot(N, V), 0.0001, 1.0); // 会作为分母，需要一个偏移
    vec3 c_diffusen = get_c_diffusen(base_color, metallic);
    vec3 diffuse_contribution = get_diffuse_contribution(base_color, metallic);
    vec3 F = F_Schlick(dotNV, base_color, metallic); // 高光项的乘数

    vec3 direct_light = vec3(0.0f);
    vec3 indirect_light = vec3(0.0f);
    vec3 indirect_light_dufuse = Irradiance_SphericalHarmonics(N, SH);
    indirect_light = indirect_light_dufuse * c_diffusen;

    for (uint i = 0; i < 1; i++) {
        vec3 L;
        vec3 sun = Directional_light(light, inWorldPos, L);
        vec3 H = normalize(V + L);
        float dotNH = clamp(dot(N, H), 0.0, 1.0);
        float dotNL = clamp(dot(N, L), 0.0001, 1.0); // 会作为分母，需要一个偏移
        float dotLH = clamp(dot(L, H), 0.0, 1.0);


        float D = D_GGX(dotNH, roughness);
        // G = Geometric shadowing term (Microfacets shadowing)
        float V = V_SmithGGXCorrelated(roughness, dotNV, dotNL);
        // F = Fresnel factor (Reflectance depending on angle of incidence)

        // F 的值：代表高光（Specular）占总光照能量的比例
        // 1.0 - F 的值：代表剩余渗透进物体内部、转化为漫反射（Diffuse）的能量比例
        // vec3 F0 = mix(vec3(0.04), baseColor, metallic);
        // vec3 c_diffuse = base_color.rgb * (vec3(1.0) - F0) * (1.0 - metallic);
        // 理论上是什么?   但是会导致 (1.0 - metallic) 的双重扣除,所以是有问题的


        // 其实全部光的
        // 漫反射 其实到 这里的就结束了
        // indirectDiffuse *= occlusion;  // 环境光遮蔽  occlusion 应该怎么样获取或提前计算
        // 球谐函数部分的算法
        // vec3 irradiance = computeSH(worldNormal);
        // vec3 indirectDiffuse = kd * irradiance * baseColor;
        // indirectDiffuse 间接漫反射

        // BRDF 的本质是一个“比例系数”：它回答的是“如果表面接收到了 100 个光子，有多少比例的光子会飞向眼睛？”
        // 它自己不代表能量。要算最终屏幕上的像素亮度（辐照度 Lo，你必须用这个比例乘以表面实际接收到的总光子数。
        // 一束平行光（如太阳光）打在桌面上，桌面宏观上能接收到多少光子？
        // 根据经典的朗伯余弦定律，光线越斜，光斑铺得越大，单位面积接收到的光子就越少

        // 这里其实并没有把遮挡算进去
        vec3 specular_contribution = D * V * F;
        direct_light += sun * (diffuse_contribution + specular_contribution) * dotNL;
    }
    vec3 out_color = emissive_color + direct_light + indirect_light;

    outFragColor_B8G8R8A8_SRGB = vec4(out_color, 1.0);
    // 好像看起来差不多了，边缘的颜色随着 物体的旋转变换很快，不应该这么快
}

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
layout (location = 6) flat in uint  material_index;
layout (location = 7) flat in uint  instance_index;

// 在前向渲染管线中，直接传递 worldPos 几乎总是更好的选择

layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;




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
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0, 1), 5.0);
    return F;
}

// Specular BRDF composition --------------------------------------------


vec3 get_BRDF(float dotNV, float dotNL, float dotLH, float dotNH, float D, float G, vec3 F) {
    vec3 color = vec3(0.0);
    vec3 lightColor = vec3(1.0);
    vec3 spec = D * F * G / (4.0 * dotNL * dotNV);
    color += spec * dotNL * lightColor;
    return color;
    // 法线与半程向量非常接近时会出现高光                 形成一个极亮、极小的高光点（类似太阳在镜子里的倒影）
    // dotNL  dotNV 都接近 90度时，也就是值都接近于零时   在物体轮廓边缘产生一道极亮的“银边”
}



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


vec3 get_normal(ShaderMaterial material,vec3 world_pos, vec3 inNormal, vec2 inUV) {
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


    float roughness = get_Roughness(material[material_index], inUV);
    float metallic = get_Metallic(material[material_index], inUV);
    vec3 base_color = get_base_color(material[material_index], inUV).rgb;
    vec3 finalEmissive = get_emissive_color(material[material_index], inUV).rgb;
    // 是否先获取无所谓，编译器会优化

    // 2. 通过 SH 函数计算当前法线方向受到的环境光辐射
    // 这个函数返回的是该方向上的预集成光照
    //    vec3 irradiance = computeSH(worldNormal);

    // 3. 最终环境漫反射颜色
    // 注意：标准的 SH 预计算通常已经把 1/PI 包含在系数里了，所以这里直接乘
    //    vec3 indirectDiffuse = irradiance * c_diff;

    // 4. 应用 AO（环境遮蔽）
    //    indirectDiffuse *= occlusion;  // 不需要dotNL


    // 3. Lambert 漫反射计算

    vec3 N = get_normal( material[material_index], inWorldPos, inNormal, inUV);
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0001, 1.0); // 会作为分母，需要一个偏移
    float dotNL = clamp(dot(N, L), 0.0001, 1.0); // 会作为分母，需要一个偏移
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float D = D_GGX(dotNH, roughness);
    // G = Geometric shadowing term (Microfacets shadowing)
    float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
    // F = Fresnel factor (Reflectance depending on angle of incidence)
    vec3 F = F_Schlick(dotNV, base_color, metallic);


    vec3 k_d = (1 - F) * (1.0 - metallic);
    vec3 f_lambert = base_color.rgb / 3.14159265359;
    vec3 indirectDiffuse = k_d * f_lambert;
    // indirectDiffuse *= occlusion;  // 环境光遮蔽  occlusion 应该怎么样获取或提前计算
    // 球谐函数部分的算法
    // vec3 irradiance = computeSH(worldNormal);
    // vec3 indirectDiffuse = kd * irradiance * baseColor;
    // indirectDiffuse 间接漫反射

    // 这里其实并没有把遮挡算进去
    vec3 finalSpecular = get_BRDF(dotNV, dotNL, dotLH, dotNH, D, G, F);

    vec3 out_color = finalEmissive + indirectDiffuse + finalSpecular + vec3(0.3,0.3,0.3);

    outFragColor_B8G8R8A8_SRGB = vec4(out_color, 1.0);
    // 好像看起来差不多了，边缘的颜色随着 物体的旋转变换很快，不应该这么快
}

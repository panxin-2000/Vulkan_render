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
// 在当前材质粗糙度下，有多少比例的“微表面”刚好把光线反射到你的眼睛里
// 本质的结果是一个 概率 的近似
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}

// Geometric Shadowing function --------------------------------------
// 在当前粗糙度下，因为微表面自身的“凹凸不平”，有多少光线会被旁边的微小结构给“遮挡”住
// 微观的情况下 , 宏观的 还需要重新计算
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

// Fresnel function ----------------------------------------------------
// 光线在两种不同介质的交界面上，有多少比例的光被【镜面反射】（Specular）回去了
// 在真实世界中，一个物体的反射率并不是固定的，而是随着你的观察角度（视角）变化而变化：
//     垂直看（反射弱）：当你垂直看着一汪清水或一块玻璃时（入射角为 0°），
//                    你能轻易看清甚至穿透它们，此时镜面反射最弱（水面只有约 2% 的光被反射）。
//     斜着看（反射强）：当你几乎平行于水面或侧面看玻璃边缘时（掠射角，入射角接近 90°），
//                    水面或玻璃会变成一面完美的镜子，此时镜面反射率暴增到 100%。
// Fresnel-Schlick 公式计算的，正是这种“越往边缘看，镜面反射越强烈”的动态比例。
vec3 F_Schlick(float cosTheta, vec3 baseColor, float metallic)
{
    vec3 F0 = mix(vec3(0.04), baseColor, metallic); //  基础反射率
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0, 1), 5.0);
    return F;
}

// Specular BRDF composition --------------------------------------------


vec3 function_specular(float dotNV, float dotNL, float D, float G, vec3 F) {
    return D * F * G / (4.0 * dotNL * dotNV);
    // F  给出了 光滑的 情况下 反射到这个方向的能量
    // D 给出 F 之后 因为粗糙度 还有多少到 需要的方向
    // G 给出了 因为 微观 遮挡 还剩余多少

    // 光源的投影拉伸
    // dotNL 朗伯余弦定律（Lambert's Cosine Law）
    // 当一束手电筒的光垂直打在墙上时，光斑很小很亮；当手电筒斜着打在墙上时，光斑会被拉长、面积变大，导致单位面积内的光子数量（光照强度）变稀疏了
    // 分母上的 dotNL 作为一个修正项，就是为了抵消光线斜射时所带来的宏观表面积增大、光线被稀释的几何效应
    //
    // 视角的立体角变换
    // 当你从宏观去看一个表面时，你眼睛（或者相机像素）所看到的，实际上是一个宏观的平坦区域。
    // 但在这个区域内部，微表面是高低起伏的，它们的真实总面积其实比宏观面积大得多。
    // 微表面理论里的 D 计算的是微观空间下的微表面面积密度（相对于微观总面积的比例）。但是，我们最终是要把它画在屏幕的宏观像素上
    // 从你的眼睛（视角 V ）看过去，宏观表面和微观表面之间存在一个空间立体角（Solid Angle）的几何投影转换。
    // dotNV 就是用来完成这个“微观空间 --> 宏观视角”转换的缩放因子。

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



    float roughness = get_Roughness(material[material_index], inUV);
    float metallic = get_Metallic(material[material_index], inUV);
    vec3 base_color = get_base_color(material[material_index], inUV).rgb;
    vec3 finalEmissive = get_emissive_color(material[material_index], inUV).rgb;
    vec3 N = get_normal(material[material_index], inWorldPos, inNormal, inUV);
    vec3 L;
    vec3 sun = Directional_light(light, inWorldPos, L);
    vec3 V = normalize(inViewVec);
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0001, 1.0); // 会作为分母，需要一个偏移
    float dotNL = clamp(dot(N, L), 0.0001, 1.0); // 会作为分母，需要一个偏移
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

/**
*
**/

    float D = D_GGX(dotNH, roughness);
    // G = Geometric shadowing term (Microfacets shadowing)
    float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
    // F = Fresnel factor (Reflectance depending on angle of incidence)
    vec3 F = F_Schlick(dotNV, base_color, metallic); // 高光项的乘数

    // F 的值：代表高光（Specular）占总光照能量的比例
    // 1.0 - F 的值：代表剩余渗透进物体内部、转化为漫反射（Diffuse）的能量比例

    vec3 k_s = F;
    vec3 c_diffuse = base_color.rgb * (vec3(1.0) - 0.04) * (1.0 - metallic);
    // vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    // vec3 c_diffuse = base_color.rgb * (vec3(1.0) - F0) * (1.0 - metallic);
    // 理论上是什么?   但是会导致 (1.0 - metallic) 的双重扣除,所以是有问题的


    vec3 Diffuse_contribution = c_diffuse / 3.14159265359; // 基础 Lambert 漫反射
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
    vec3 specular_contribution = sun * function_specular(dotNV, dotNL, D, G, F);

    vec3 out_color = finalEmissive + sun * (Diffuse_contribution + specular_contribution) * dotNL;

    outFragColor_B8G8R8A8_SRGB = vec4(out_color, 1.0);
    // 好像看起来差不多了，边缘的颜色随着 物体的旋转变换很快，不应该这么快
}

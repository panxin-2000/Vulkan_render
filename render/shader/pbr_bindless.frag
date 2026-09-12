/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"
#include "pbr_material.glsl"




// 在前向渲染管线中，直接传递 worldPos 几乎总是更好的选择


layout (set = 3, binding = 0) readonly buffer model_material_parameters {
    uint material_pbr_index[];
};

#define DISTANCE_FOG 1

#if defined(PASS_COLOR)
layout (location = 0) out vec4 outFragColor_R16G16B16A16_SFLOAT;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inViewVec;
layout (location = 3) in vec4 inShadowCoord;
layout (location = 4) in vec3 inWorldPos;
layout (location = 5) flat in uint material_index;
layout (location = 6) flat in uint instance_index;

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
    vec3 normalSample = texture(sampler2D(bindless_texture2D[material.normalTexture], bindless_samplers[0]), inUV).rgb;
    // 2. 解码到 [-1, 1] 范围
    // 公式：n = color * 2.0 - 1.0
    vec3 tangent_space_Normal = normalSample * 2.0 - 1.0;

    vec3 N_object = normalize(inNormal);
    mat3 TBN = ComputeTBNMatrix(world_pos, N_object, inUV);
    vec3 N = TBN * normalize(tangent_space_Normal);
    return N;

}

float textureProj(const highp sampler2DArray shadow_texture, vec4 shadowCoord, vec2 offset, uint cascadeIndex, float bias)
{
    float shadow = 1.0;

    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {
        float dist = texture(shadow_texture, vec3(shadowCoord.st + offset, cascadeIndex)).r;
        // 如果从光源看过去的最近距离 dist，小于当前像素的距离 shadowCoord.z，说明前面有物体挡住了光
        // 下面新加的这一行是有用的
        if (dist > shadowCoord.z + bias) {
            shadow = 0.0f;
        }
    }
    return shadow;

}

float filterPCF(const highp sampler2DArray shadow_texture, vec4 sc, uint cascadeIndex, float bias)
{
    ivec2 texDim = textureSize(shadow_texture, 0).xy;
    float scale = 0.75;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            shadowFactor += textureProj(shadow_texture, sc, vec2(dx * x, dy * y), cascadeIndex, bias);
            count++;
        }
    }
    return shadowFactor / count;
}

float GetLinearViewDepth()
{
    float z = invProjection[2][2] * gl_FragCoord.z + invProjection[3][2];
    float w = invProjection[2][3] * gl_FragCoord.z + invProjection[3][3];
    return z / w;
}




const mat4 biasMat = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0);



float get_shadow(const highp sampler2DArray shadow_texture, vec3 normal, vec3 lightDir, vec3  WorldPos){

    // Depth compare for shadowing // Clip Space
    vec4 view_pos = view * vec4(inWorldPos, 1.0);
    float viewDepth = -view_pos.z;

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    //    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    uint cascadeIndex = 0;


    for (uint i = 0; i < SHADOW_MAP_CASCADE_COUNT; ++i) {
        if (viewDepth > cascadeSplits[i]) {
            cascadeIndex = i + 1;
        }
    }
    //    bias *= (10.0 / cascadeSplits[cascadeIndex]);

    float currentSplit = cascadeSplits[cascadeIndex];
    uint prev = (cascadeIndex == 0) ? 0 : cascadeIndex - 1;

    float prevSplit = (cascadeIndex == 0) ? 0.0 : cascadeSplits[cascadeIndex - 1];

    float blendBand = (currentSplit - prevSplit) * 0.1;
    float blendDist = currentSplit - viewDepth;

    vec4 shadowCoord = biasMat * cascadeViewProjMat[cascadeIndex] * vec4(inWorldPos, 1.0);

    // NDC 空间 shadowCoord / shadowCoord.w
    float shadow = 0;
    //    if (enablePCF == 1) {
    shadow = filterPCF(shadow_texture, shadowCoord / shadowCoord.w, cascadeIndex, bias);
    //    } else {
    //    shadow = textureProj(shadow_texture, shadowCoord / shadowCoord.w, vec2(0.0), cascadeIndex);
    //    }


    // 4. 如果在边界内，且存在下一层级，则进行混合采样
    if (blendDist < blendBand && cascadeIndex < 3) {
        // 计算混合权重 (0.0 完全属于当前层，1.0 完全属于下一层)
        float alpha = 1.0 - (blendDist / blendBand);
        vec4 shadowCoord = biasMat * cascadeViewProjMat[cascadeIndex + 1] * vec4(inWorldPos, 1.0);
        float  shadowNext = filterPCF(shadow_texture, shadowCoord / shadowCoord.w, cascadeIndex + 1, bias);
        // 线性混合两层阴影结果
        return mix(shadow, shadowNext, alpha);
    }

    return shadow;
}

void main()
{

    // 是否先获取无所谓，编译器会优化
    //

    float AO = texture(global_Blur_SSAO, (gl_FragCoord.xy / screen_size.xy)).r;

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
    float shadow = get_shadow(global_shadow_texture, N, -normalize(light.rotate.xyz), inWorldPos);

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
        direct_light += sun * (diffuse_contribution * (vec3(1.0) - F) + specular_contribution) * dotNL * shadow;
    }
    vec3 out_color = emissive_color + direct_light + indirect_light * AO;

    #if defined(DISTANCE_FOG)
    float distance = length(viewPos.xyz - inWorldPos.xyz);
    float fogFactor = get_fog_factor(distance, fogStart, fogEnd, fogDensity, fogType);
    out_color = mix(fogColor.rgb, out_color.rgb, fogFactor);
    #endif

    outFragColor_R16G16B16A16_SFLOAT = vec4(out_color, 1.0);
    // 好像看起来差不多了，边缘的颜色随着 物体的旋转变换很快，不应该这么快
}

#elif defined(PASS_DEPTH) || defined(PASS_SHADOW_MAP)

void main()
{

}
#elif defined(PASS_PICKUP)

layout (location = 0) flat in uint entity;

layout (location = 0) out uint out_entity_R32_UINT;
void main()
{
    out_entity_R32_UINT = entity;
}


#elif defined(PASS_RANDOM_TRIANGLE_COLOR)

layout (location = 0) out vec4 outFragColor_R16G16B16A16_SFLOAT;

void main()
{
    outFragColor_R16G16B16A16_SFLOAT = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);
}

#endif












// Specular BRDF composition --------------------------------------------




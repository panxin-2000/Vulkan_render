/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout (set = 0, binding = 4) uniform sampler2D global_samplerColorMap[];

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inLightVec;
layout (location = 3) in vec3 inViewVec;

layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;


layout (set = 1, binding = 1) uniform texture_index
{
    int index;
};


float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}

void main()
{

    vec4 inColor = texture(global_samplerColorMap[index], inUV);

    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);
    vec3 ambient = vec3(0.1);

    // vec3(0.75) 代表的是 高光颜色 (Specular Color) 的强度或因子
    // --- 原来的 Phong 逻辑 ---
    //    vec3 R = reflect(-L, N);
    //    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);

    // --- 修改后的 Blinn-Phong 逻辑 ---
    vec3 H = normalize(L + V); // 计算半程向量
    vec3 specular = pow(max(dot(N, H), 0.0), 32.0) * vec3(0.75); // 计算 N 和 H 的夹角

    vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
    outFragColor_B8G8R8A8_SRGB = vec4((ambient + diffuse) * inColor.rgb + specular, 1.0);
}

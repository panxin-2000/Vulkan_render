/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout (set = 0, binding = 0) uniform sampler2D global_samplerColorMap[];

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 in_Factor;
layout (location = 4) in vec3 in_LightVec;
layout (location = 5) in vec3 in_ViewVec;
layout (location = 6) flat in uint in_InstanceIndex;

layout (location = 0) out vec4 outFragColor;

float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}

void main()
{
    vec3 N = normalize(inNormal);
    vec3 L = normalize(in_LightVec);
    vec3 V = normalize(in_ViewVec);
    vec3 R = reflect(-L, N);
    //    vec3 diffuse = max(dot(N, L), (0.0025));
    vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);

    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
    vec4 color = texture(global_samplerColorMap[in_InstanceIndex], inUV);
    outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
    // gl_PrimitiveID 需要 VkPhysicalDeviceFeatures::geometryShader 但是有错误 Validation Error 可以不管的
}
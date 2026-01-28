/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450

layout (set = 0, binding = 0) uniform sampler2D samplerColorMap[];

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

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
    //    outFragColor = vec4(0.5, 0.6, 0.7, 1.0);
    outFragColor = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);
    // gl_PrimitiveID 需要 VkPhysicalDeviceFeatures::geometryShader 但是有错误 Validation Error 应付是可以不管的
}
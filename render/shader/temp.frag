/* Copyright (c) 2024, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
#extension GL_EXT_nonuniform_qualifier: require

//layout (set = 0, binding = 0) uniform sampler2D global_samplerColorMap[];

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inLightVec;
layout (location = 3) in vec3 inViewVec;

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

    vec3 inColor = vec3(0.8, 0.8, 0.8);
//    outFragColor = vec4(inColor.rgb, 1.0);

    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);
    vec3 R = reflect(-L, N);
    vec3 ambient = vec3(0.1);
    vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
    outFragColor = vec4((ambient + diffuse) * inColor.rgb + specular, 1.0);
}

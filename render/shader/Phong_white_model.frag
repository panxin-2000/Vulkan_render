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

layout (location = 0) out vec4 outFragColor_R16G16B16A16_SFLOAT;


void main()
{

    vec3 baseColor = vec3(0.8, 0.8, 0.8);


    //    不对，公式中缺少了太多的内容
    //    vec3 f_0_04 = vec3(0.04);
    //    vec3 specularColor = mix(f_0_04, baseColor.rgb, metallic);
    //    vec3 diffuseColor = baseColor * (1.0 - 0.04) * (1.0 - metallic);

    vec3 N = normalize(inNormal);
    vec3 L = normalize(inLightVec);
    vec3 V = normalize(inViewVec);
    vec3 R = reflect(-L, N);
    vec3 ambient = vec3(0.1);
    vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
    vec3 specular = pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75);
    outFragColor_R16G16B16A16_SFLOAT = vec4((ambient + diffuse) * baseColor.rgb + specular, 1.0);
}

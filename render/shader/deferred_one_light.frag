#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (set = 2, binding = 1) uniform sampler2D samplerposition;
layout (set = 2, binding = 2) uniform sampler2D samplerNormal;
layout (set = 2, binding = 3) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor_B8G8R8A8_SRGB;



void main()
{
    // Get G-Buffer values
    vec3 world_pos = texture(samplerposition, inUV).rgb;
    vec3 normal = texture(samplerNormal, inUV).rgb;
    vec4 Base_color = texture(samplerAlbedo, inUV);


    vec3 inLightVec = lightPos.xyz - world_pos.xyz;
    vec3 inViewVec = viewPos.xyz - world_pos.xyz;

    vec3 N = normalize(normal);
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
    outFragcolor_B8G8R8A8_SRGB = vec4((ambient + diffuse) * Base_color.rgb + specular, 1.0);
}
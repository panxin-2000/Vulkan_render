#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (set = 2, binding = 1) uniform sampler2D samplerPosition;
layout (set = 2, binding = 2) uniform sampler2D samplerNormal;
layout (set = 2, binding = 3) uniform sampler2D samplerBaseColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;


layout (set = 2, std140, binding = 4) readonly buffer light_buffer {
    Light lights[];
};

// 这里其实多了一个要求，具有相同名字的需要有相同的偏移
layout (push_constant) uniform PushConstants {
    vec2 frag_scale;
    vec2 frag_translate;
    vec2 frag_dsdf;
} pushConstants;

void main()
{

    // world_pos 可以反向 计算出来 ， 代码如下 ，减少 GPU 带宽 的压力
    // // 1. Get the depth value from the depth attachment
    // float z = texture(depthSampler, inUV).r;
    //
    // // 2. Convert UV and Z to Normalized Device Coordinates (NDC)
    // // Vulkan NDC: x,y in [-1, 1], z in [0, 1]
    // vec4 clipPos = vec4(inUV * 2.0 - 1.0, z, 1.0);
    //
    // // 3. Transform from Clip Space to World Space
    // vec4 worldPos = pc.invViewProj * clipPos;
    //
    // // 4. Perspective Division
    // worldPos /= worldPos.w;

    // Get G-Buffer values
    vec3 world_pos = texture(samplerPosition, inUV).rgb;
    vec3 normal = texture(samplerNormal, inUV).rgb;
    vec4 Base_color = texture(samplerBaseColor, inUV);

    vec3 LightVec = lightPos.xyz - world_pos.xyz;
    vec3 ViewVec = viewPos.xyz - world_pos.xyz;

    vec3 N = normalize(normal);
    vec3 L = normalize(LightVec);
    vec3 V = normalize(ViewVec);

    vec3 ambient = vec3(0.1);

    vec3 H = normalize(L + V); // 计算半程向量
    vec3 specular = pow(max(dot(N, H), 0.0), 32.0) * vec3(0.75); // 计算 N 和 H 的夹角

    vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
    outFragColor_B8G8R8A8_SRGB = vec4((ambient + diffuse) * Base_color.rgb + specular, 1.0);
    //    outFragColor_B8G8R8A8_SRGB = vec4(normal.rgb, 1.0);
}
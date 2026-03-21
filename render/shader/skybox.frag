#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;



layout (set = 2, binding = 1) uniform samplerCube sampler_skybox;

void main()
{
    vec3 color = (texture(sampler_skybox, inUVW)).rgb;
    outFragColor_B8G8R8A8_SRGB = vec4(color.rgb, 1.0);
}
#version 450

#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;



layout (push_constant) uniform PushConstants {
    uint input_offscreen;
} pushConstants;

void main()
{
    vec4 color = texture(bindless_samplerColorMap[input_offscreen], inUV);
    outFragColor_B8G8R8A8_SRGB = vec4(color, 1.0);
}
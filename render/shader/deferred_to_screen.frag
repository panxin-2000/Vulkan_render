#version 450

#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;



void main()
{
    float color = texture(global_depth, inUV).r;
    outFragColor_B8G8R8A8_SRGB = vec4(color, color, color, 1.0);
}
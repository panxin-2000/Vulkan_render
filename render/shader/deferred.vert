#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


layout (location = 0) out vec2 outUV;




layout (set = 1, binding = 0) uniform model_4x4
{
    mat4 model;
};

layout (push_constant) uniform PushConstants {
    vec2 scale;
    vec2 translate;
    vec2 dsdf;
} pushConstants;

void main()
{
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}
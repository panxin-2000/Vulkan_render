#version 450

layout (location = 0) out vec2 outUV;


layout (set = 0, binding = 0) uniform global_view_4x4
{
    mat4 view;
};
layout (set = 0, binding = 1) uniform global_projection_4x4
{
    mat4 projection;
};
layout (set = 0, binding = 2) uniform global_world_light_Pos
{
    vec3 lightPos;
};
layout (set = 0, binding = 3) uniform global_world_view_Pos
{
    vec3 viewPos;
};

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
#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

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

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outWorldPos;

void main()
{

    gl_Position = projection * view * model * vec4(inPos.xyz, 1.0);
    outUV = inUV;
    outWorldPos = vec3(model * vec4(inPos.xyz, 1.0));
    mat3 mNormal = transpose(inverse(mat3(model)));
    outNormal = mNormal * normalize(inNormal);
}

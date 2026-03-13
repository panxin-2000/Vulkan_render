//
// Created by 潘鑫 on 2026/3/13.
//

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
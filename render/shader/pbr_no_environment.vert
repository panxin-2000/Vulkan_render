#version 450


#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 2, binding = 0) uniform model_4x4
{
    mat4 model;
};

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;

layout (push_constant) uniform PushConsts {
    vec3 objPos;
} pushConsts;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec3 locPos = vec3(model * vec4(inPos, 1.0));
    outWorldPos = locPos + pushConsts.objPos;         // 获取位置的偏移
    outNormal = mat3(model) * inNormal;           // 法线的方向并不会因为 位置的偏移 而改变，只会因为旋转而改变
    gl_Position = projection * view * vec4(outWorldPos, 1.0);
}

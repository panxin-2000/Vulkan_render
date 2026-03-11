#version 450

layout (set = 1, binding = 1) uniform sampler2D samplerPosition;
layout (set = 1, binding = 2) uniform sampler2D samplerNormal;
layout (set = 1, binding = 3) uniform sampler2D samplerBaseColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};

layout (set = 1, binding = 4) uniform UBO
{
    Light lights[6];
    vec4 viewPos;
    int displayDebugTarget;
} ubo;

void main()
{
    // Get G-Buffer values
    vec3 world_pos = texture(samplerPosition, inUV).rgb;
    vec3 normal = texture(samplerNormal, inUV).rgb;
    vec4 Base_color = texture(samplerBaseColor, inUV);


    outFragcolor = vec4(Base_color.xyz, 1.0);
}
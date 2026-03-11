#version 450

layout (binding = 1) uniform sampler2D samplerposition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};

layout (binding = 4) uniform UBO
{
    Light lights[6];
    vec4 viewPos;
    int displayDebugTarget;
} ubo;

void main()
{
    // Get G-Buffer values
    //    vec3 world_pos = texture(samplerposition, inUV).rgb;
    //    vec3 normal = texture(samplerNormal, inUV).rgb;
    //    vec4 Base_color = texture(samplerAlbedo, inUV);


    outFragcolor = vec4(0.7, 0.7, 0.7, 1.0);
}
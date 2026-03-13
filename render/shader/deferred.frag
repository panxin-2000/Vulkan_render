#version 450

layout (set = 1, binding = 1) uniform sampler2D samplerPosition;
layout (set = 1, binding = 2) uniform sampler2D samplerNormal;
layout (set = 1, binding = 3) uniform sampler2D samplerBaseColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor_B8G8R8A8_SRGB;

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};



layout (set = 1, std140, binding = 4) readonly buffer light_buffer {
    Light lights[];
};

// 这里其实多了一个要求，具有相同名字的需要有相同的偏移
layout (push_constant) uniform PushConstants {
    vec2 frag_scale;
    vec2 frag_translate;
    vec2 frag_dsdf;
} pushConstants;

void main()
{
    // Get G-Buffer values
    vec3 world_pos = texture(samplerPosition, inUV).rgb;
    vec3 normal = texture(samplerNormal, inUV).rgb;
    vec4 Base_color = texture(samplerBaseColor, inUV);


    outFragcolor_B8G8R8A8_SRGB = vec4(Base_color.xyz, 1.0);
}
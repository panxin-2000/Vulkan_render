#version 450 core
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;

layout (set = 0, binding = 0) uniform sampler2D sTexture;

layout (location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

void main()
{
    outFragColor_B8G8R8A8_SRGB = In.Color * texture(sTexture, In.UV.st);
}

#version 450
#extension GL_GOOGLE_include_directive: require

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;



layout (set = 1, binding = 1) uniform samplerCube sampler_skybox;

void main()
{
    vec3 color = (texture(sampler_skybox, inUVW)).rgb;
    outColor = vec4(color.rgb, 1.0);
}
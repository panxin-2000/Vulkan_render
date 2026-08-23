#version 450

layout (binding = 0) uniform sampler2D samplerSSAO;

layout (location = 0) in vec2 inUV;

layout (location = 0) out float out_occlusion_R8_UNORM;


#include "global_shader_common.glsl"

#include "geometry.glsl"


void main()
{
    const int blurRange = 2;
    int n = 0;
    vec2 texelSize = 1.0 / vec2(textureSize(samplerSSAO, 0));
    float result = 0.0;
    for (int x = -blurRange; x <= blurRange; x++)
    {
        for (int y = -blurRange; y <= blurRange; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(samplerSSAO, inUV + offset).r;
            n++;
        }
    }
    // 这里的模糊代码,应该是还是有更好的解决的方案的
    out_occlusion_R8_UNORM = result / (float(n));
}
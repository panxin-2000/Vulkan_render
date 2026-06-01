#version 450
#extension GL_EXT_nonuniform_qualifier: require
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}

float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}


/**
* inout uint state 需要能够被输入和输出，最开始的输入不能是零
* 一个默认值 是 static thread_local uint32_t state = 2463534242u;
* 最重要的是应该是在01之间是均匀的
**/
float xorshift32_fastest(inout uint state) {
    state ^= state << 13u;
    state ^= state >> 17u;
    state ^= state << 5u;
    // IEEE 754 float manipulation:
    // Mask out 23 bits for mantissa, set exponent bits to match the 1.0 to 2.0 range
    uint m = (state >> 9u) | 0x3F800000u;
    float f = uintBitsToFloat(m); // Interprets bits directly as a float [1.0, 2.0)
    return f - 1.0;               // Shifts range to [0.0, 1.0)
}


void main()
{
    outFragColor_B8G8R8A8_SRGB = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);
}
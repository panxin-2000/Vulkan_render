#version 450
#extension GL_EXT_nonuniform_qualifier: require



layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}

float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}

void main()
{
    outFragColor_B8G8R8A8_SRGB = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);
}
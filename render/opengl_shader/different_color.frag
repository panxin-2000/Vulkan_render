#version 330 core
out vec4 FragColor;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}

float hash(int xy) {
    uint x = uint(xy);
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = ((x >> 16u) ^ x) * 0x45d9f3b3u;
    x = (x >> 16u) ^ x;
    return float(x) / 4294967295.0;
}

uniform sampler2D ourTexture1;
in vec2 UV_position_to_fragment;


void main()
{
    FragColor = texture(ourTexture1, UV_position_to_fragment);
//    FragColor = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);
}
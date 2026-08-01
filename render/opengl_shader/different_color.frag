#version 330 core
out vec4 FragColor;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}


uniform sampler2D ourTexture1;
in vec2 UV_position_to_fragment;


void main()
{
    FragColor = texture(ourTexture1, UV_position_to_fragment);
//    FragColor = vec4(hash(gl_PrimitiveID + 1), hash(gl_PrimitiveID + 2), hash(gl_PrimitiveID + 3), 1.0);
}
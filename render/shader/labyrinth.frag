#version 330 core
out vec4 FragColor;
in vec3 COLOR;
in vec2 UV_position_to_fragment;

uniform sampler2D ourTexture1;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}
void main()
{
    int triID = gl_PrimitiveID;
    //    FragColor = texture(ourTexture1, UV_position_to_fragment);
    FragColor = vec4(COLOR, 1.0);
}
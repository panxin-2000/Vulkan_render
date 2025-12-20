#version 330 core
out vec4 FragColor;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}
void main()
{
    int triID = gl_PrimitiveID;
    FragColor = vec4(rand(triID), rand(triID + 1), rand(triID + 2), 1.0);
}
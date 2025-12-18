#version 330 core
out vec4 FragColor;
in vec3 COLOR;
float rand(int seed) {return fract(sin(float(seed)) * 43758.5453);}
void main()
{
    int triID = gl_PrimitiveID;
    FragColor = vec4(COLOR, 1.0);
}
#version 330 core
layout (location = 0) in vec3 aPos;
//uniform float value;

void main()
{
    float value = 1.0 / 8.0;
    gl_Position = vec4(aPos.x * value, aPos.y * value, aPos.z * value, 1.0);
}
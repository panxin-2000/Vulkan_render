#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
out vec3 COLOR;
uniform float value;
void main()
{
    gl_Position = vec4(aPos.x * value, aPos.y * value, aPos.z, 1.0);
    COLOR = color;
}
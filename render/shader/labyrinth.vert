#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 UV_position;
out vec3 COLOR;
out vec2 UV_position_to_fragment;
uniform float value;
void main()
{
    gl_Position = vec4(aPos.x * value, aPos.y * value, aPos.z, 1.0);
    COLOR = color;
    UV_position_to_fragment = UV_position;
}
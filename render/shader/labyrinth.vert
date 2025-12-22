#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 UV_position;
out vec3 COLOR;
out vec2 UV_position_to_fragment;
uniform mat3 value;
void main()
{
    vec3 pos = value * aPos;
    gl_Position = vec4(pos, 1.0);
    COLOR = color;
    UV_position_to_fragment = UV_position;
}
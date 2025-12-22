#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 UV_position;
out vec3 COLOR;
out vec2 UV_position_to_fragment;
uniform mat4 model_transform;
void main()
{
    vec4 pos = model_transform * vec4(aPos, 1.0);
    gl_Position = pos;
    COLOR = color;
    UV_position_to_fragment = UV_position;
}
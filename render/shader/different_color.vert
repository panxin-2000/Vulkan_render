#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 UV_position;

//uniform float value;

uniform mat4 model_transform;  // 2d的 大小。
out vec2 UV_position_to_fragment;

void main()
{
    vec4 pos = model_transform * vec4(aPos, 1.0);
    gl_Position = pos;
    UV_position_to_fragment = UV_position;

    //    gl_Position = vec4(aPos, 1.0);
}
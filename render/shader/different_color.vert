#version 330 core
layout (location = 0) in vec3 aPos;
//uniform float value;

uniform mat4 model_transform;  // 2d的 大小。

void main()
{
    vec4 pos = model_transform * vec4(aPos, 1.0);
    gl_Position = pos;
}
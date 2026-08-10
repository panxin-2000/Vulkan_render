#version 450

// 输入：从顶点着色器传过来的颜色
layout (location = 0) in vec4 fragColor;

// 输出：最终的像素颜色
layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;

void main() {
    outFragColor_B8G8R8A8_SRGB = fragColor;
}

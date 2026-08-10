#version 450

#extension GL_EXT_shader_explicit_arithmetic_types_int8: require

// 输入：顶点坐标 (location = 0) 和 颜色 (location = 1)
layout (location = 0) in vec2 inPosition;
layout (location = 1) in u8vec4 inColor;

// 输出：传递给片元着色器的颜色
layout (location = 0) out vec4 fragColor;


layout (push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
} pc;


void main() {
    // 直接输出二维坐标，Z为0，W为1
    gl_Position = vec4(inPosition * pc.uScale + pc.uTranslate, 0, 1);
    fragColor = inColor.xyz * (1.0 / 255.0);
}

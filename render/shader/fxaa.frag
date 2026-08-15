#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"

#define FXAA_PC 1          // 声明当前是 PC 平台（非主机、非移动端）
#define FXAA_GLSL_130 1    // 声明使用现代 GLSL 语法
#define FXAA_QUALITY__PRESET 12 // 选择画质预设（12 是官方推荐的中高性价比画质）

// --- 第二步：引入 NVIDIA 官方头文件 ---
#include "Fxaa3_11.h"

layout (location = 0) in vec2 inUV;


uniform sampler2D u_MainTex;   // 需要抗锯齿的场景纹理
uniform vec4 u_FxaaRcpFrame;   // 🌟 核心参数：屏幕分辨率的倒数

layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;


void main() {
    // --- 第四步：构建 FxaaPixelShader 所需的全部复杂参数 ---

    FxaaTex tex;
    tex.smpl = u_MainTex; // 绑定纹理

    // 调用官方核心函数
    outFragColor_B8G8R8A8_SRGB
    = FxaaPixelShader(
        inUV, // pos: 当前像素的 UV 坐标
        vec4(0.0), // fxaaConsolePosPos: 主机专用，PC 填 0
        tex, // tex: 绑定的纹理结构体
        tex, // fxaaConsole360TexLuma: 远古 Xbox360 专用，同上
        tex, // fxaaConsole360TexChroma: 同上
        u_FxaaRcpFrame.xy, // fxaaQualityRcpFrame: 核心！1.0 / 屏幕分辨率
        vec4(0.0), // fxaaConsoleRcpFrameOpt: 主机专用，填 0
        vec4(0.0), // fxaaConsoleRcpFrameOpt2: 主机专用，填 0
        vec4(0.0), // fxaaConsole360RcpFrameOpt2: 主机专用，填 0
        0.75, // fxaaQualitySubpix: 子像素过滤程度 (0.0~1.0)
        0.166, // fxaaQualityEdgeThreshold: 边缘检测阈值
        0.0833, // fxaaQualityEdgeThresholdMin: 暗部裁剪阈值
        0.0, // fxaaConsoleEdgeSharpness: 主机专用
        0.0, // fxaaConsoleEdgeThreshold: 主机专用
        0.0, // fxaaConsoleEdgeThresholdMin: 主机专用
        vec4(0.0)                   // fxaaConsole360ConstDir: 主机专用
    );
}

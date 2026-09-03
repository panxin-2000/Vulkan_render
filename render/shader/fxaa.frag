#version 450
#extension GL_GOOGLE_include_directive: enable
#include "global_shader_common.glsl"


#define texture2D(t, uv)              texture(t, uv)
#define texture2DLod(t, uv, lod)      textureLod(t, uv, lod)

#define FXAA_PC 1          // 声明当前是 PC 平台（非主机、非移动端）
#define FXAA_GLSL_130 1    // 声明使用现代 GLSL 语法
#define FXAA_QUALITY__PRESET 12 // 选择画质预设（12 是官方推荐的中高性价比画质）
#define FXAA_GREEN_AS_LUMA 1

// --- 第二步：引入 NVIDIA 官方头文件 ---
#include "Fxaa3_11.h"

layout (location = 0) in vec2 inUV;



layout (location = 0) out vec4 outFragColor_B8G8R8A8_SRGB;


void main() {
    ivec2 texSize = textureSize(global_offscreen, 0);

    vec4 fxaa_result = FxaaPixelShader(
            inUV, // pos: 当前像素的 UV 坐标
            vec4(0.0), // fxaaConsolePosPos: 主机专用，PC 填 0
            global_offscreen, // tex: 绑定的纹理结构体
            global_offscreen, // fxaaConsole360TexLuma: 远古 Xbox360 专用，同上
            global_offscreen, // fxaaConsole360TexChroma: 同上
            1.0 / screen_size.xy, // fxaaQualityRcpFrame: 核心！1.0 / 屏幕分辨率
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
    // Unsharp Mask
    // 还是需要一个单独的pass的 

    // 暗角
    vec3 finalColor = apply_vignette(fxaa_result.rgb, inUV, camera_vignette_intensity, camera_vignette_smoothness);
    // 模拟胶片
    {
        float noise = get_noise_v2(uvec2(inUV * texSize), render_timeline);
        float grainShift = noise - 0.5; // 重映射到 [-0.5, 0.5]

        float luma = dot(finalColor, vec3(0.2126, 0.7152, 0.0722));
        float lumaMask = 4.0 * luma * (1.0 - luma);
        lumaMask = clamp(lumaMask, 0.0, 1.0);
        float offset = film_grain_intensity * grainShift * lumaMask;
        finalColor = finalColor * (1.0 + offset);
    }
    outFragColor_B8G8R8A8_SRGB = vec4(finalColor, fxaa_result.a);
}

//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_SHADER_COMMON_H
#define HELLO_MAC_SHADER_COMMON_H

#include <volk.h>
#include "base_geometry/base.h"

// 渲染层级（控制绘制顺序，如UI > 角色 > 场景）
enum class RenderLayer {
    Background, // 背景
    World,      // 场景物体
    Character,  // 角色
    Weapon,     // 武器
    UI          // 界面
};

// 混合模式（透明/不透明渲染）
enum class BlendMode {
    Opaque,     // 不透明（默认）
    AlphaBlend, // 阿尔法混合（普通透明）
    Additive,   // 加法混合（发光效果）
    Multiply    // 乘法混合（暗化效果）
};

// 阴影模式
enum class ShadowMode {
    CastAndReceive, // 投射并接收阴影（默认）
    CastOnly,       // 仅投射阴影
    ReceiveOnly,    // 仅接收阴影
    None            // 无阴影
};


struct shader_and_share {
    VkShaderModule shader;
    uint16_t shared_number;
};

struct texture_and_share {
    unsigned int texture;
    uint16_t shared_number;
};

#ifdef WITH_VULKAN_BACKEND

// 之后下面的内容还是需要转移的
// #include "vulkan_buffer.h"
// Vulkan 的核心目标是“零隐式开销”。
// 预计算：当你创建 VkPipeline 时，驱动程序会针对你指定的拓扑结构、顶点格式和着色器进行“整体优化编译”
// 所以拓扑结构不在这里，而在管线中
// 某些着色器阶段对拓扑结构有严格的要求
// 倾向于为不同的拓扑结构预创建不同的 Pipeline


#endif


struct pipeline_and_share {
#ifdef WITH_VULKAN_BACKEND
    VkPipeline pipeline = VK_NULL_HANDLE;
#elif  WITH_OPENGL_BACKEND
    unsigned int buffer;
#endif
    uint16_t shared_number = 0;
};


struct pos_struct {
    float x, y, z;
};

struct normal_struct {
    float a, b, c;
};

struct uv_struct {
    float u, v;
};


struct Vertex {
    Point_3 pos;
    Point_3 normal{0, 0, 1};
    Point_2 uv{0, 0};
};

struct Line {
    Point_2 pos;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    uint8_t color_a;
};


struct Vertex_2D {
    Point_2 pos;
    uv_struct uv{0, 0};
};

struct Vertex_imgui {
    Point_2 pos;
    uv_struct uv{0, 0};
    uint32_t color;
};

struct Picture_parameters {
    uint32_t width           = 0;
    uint32_t height          = 0;
    int channels        = 4;
    uint8_t *image_data = nullptr;
};


template<typename T>
uint32_t to_u32(T val) {
    assert(val <= std::numeric_limits<uint32_t>::max());
    return static_cast<uint32_t>(val);
}
#endif //HELLO_MAC_SHADER_COMMON_H

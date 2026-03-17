//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_SHADER_COMMON_H
#define HELLO_MAC_SHADER_COMMON_H

// 定义一个这个文件是什么？ 会被 vulkan_handle.h 引用的头文件

#include <list>
#include <volk.h>

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

enum GPUPrimType : int8_t {
    GPU_PRIM_POINTS,
    GPU_PRIM_LINES,
    GPU_PRIM_TRIS,
    GPU_PRIM_LINE_STRIP,
    GPU_PRIM_LINE_LOOP, /* GL has this, Vulkan and Metal do not */
    GPU_PRIM_TRI_STRIP,
    GPU_PRIM_TRI_FAN, /* Metal API does not support this. */

    /* Metal API does not support ADJ primitive types but
     * handled via the geometry-shader-alternative path. */
    GPU_PRIM_LINES_ADJ,
    GPU_PRIM_TRIS_ADJ,
    GPU_PRIM_LINE_STRIP_ADJ,

    GPU_PRIM_NONE,
};


struct shader_and_share {
#ifdef WITH_VULKAN_BACKEND
    VkShaderModule shader;
#elif  WITH_OPENGL_BACKEND
    unsigned int shader;
#endif

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


// 下面的应该是直接从 blender 中 复制过来的，但是没有做什么处理

struct VKViewportData {
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    // 为什么 blender 里这里写的是向量呢？

    bool operator==(const VKViewportData &other) const {
        if (viewports.size() != other.viewports.size() && scissors.size() != other.scissors.size()) {
            return false;
        }

        if (memcmp(viewports.data(), other.viewports.data(), viewports.size() * sizeof(VkViewport)) !=
            0) {
            return false;
        }

        if (memcmp(scissors.data(), other.scissors.data(), scissors.size() * sizeof(VkRect2D)) != 0) {
            return false;
        }

        return true;
    }

    bool operator!=(const VKViewportData &other) const {
        return !(*this == other);
    }
};


/** Resources bound for a compute/graphics pipeline. */
struct VKBoundPipeline {
    VkPipeline vk_pipeline;
    VkDescriptorSet vk_descriptor_set;
    VkDeviceAddress descriptor_buffer_device_address;
    VkDeviceSize descriptor_buffer_offset;
};

struct VKIndexBufferBinding {
    VkBuffer buffer;
    VkIndexType index_type;

    bool operator==(const VKIndexBufferBinding &other) const {
        return buffer == other.buffer && index_type == other.index_type;
    }

    bool operator!=(const VKIndexBufferBinding &other) const {
        return !(*this == other);
    }
};


struct VKVertexBufferBindings {
#define buffer_size 16
    uint32_t buffer_count;
    VkBuffer buffer[buffer_size];
    VkDeviceSize offset[buffer_size];

    bool operator==(const VKVertexBufferBindings &other) const {
        if (buffer_count != other.buffer_count)
            return false;
        for (uint32_t i = 0; i < buffer_size; ++i) {
            if ((buffer[i] != other.buffer[i]) ||
                (offset[i] != other.offset[i])) {
                return false;
            }
        }
        return true;
    }
#undef buffer_size
    bool operator!=(const VKVertexBufferBindings &other) const {
        return !(*this == other);
    }
};

struct VKR_bind_pipelines {
    /** Last bound resources for compute pipeline. */
    VKBoundPipeline compute;

    /** Last bound resources for graphics pipeline. */
    struct {
        VKBoundPipeline pipeline;
        VKIndexBufferBinding index_buffer;
        VKVertexBufferBindings vertex_buffers;
        VKViewportData viewport_state;
        std::optional<float> line_width;
    } graphics;
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
    pos_struct pos;
    normal_struct normal;
    uv_struct uv;
};

#endif //HELLO_MAC_SHADER_COMMON_H

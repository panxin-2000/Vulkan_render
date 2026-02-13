//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_SHADER_COMMON_H
#define HELLO_MAC_SHADER_COMMON_H

#include <vk_mem_alloc.h>
#include <base_element/point_3.h>

#include "volk.h"

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

// Vulkan 的核心目标是“零隐式开销”。
// 预计算：当你创建 VkPipeline 时，驱动程序会针对你指定的拓扑结构、顶点格式和着色器进行“整体优化编译”
// 所以拓扑结构不在这里，而在管线中
// 某些着色器阶段对拓扑结构有严格的要求
// 倾向于为不同的拓扑结构预创建不同的 Pipeline
struct Model_mesh {
    // 不做
    VkBuffer vertices_buffer          = VK_NULL_HANDLE;
    VmaAllocation vertices_allocation = VK_NULL_HANDLE;
    VkDeviceSize vertices_offset      = 0; // 以字节为单位的偏移
    VkBuffer indices_buffer           = VK_NULL_HANDLE;
    VmaAllocation indices_allocation  = VK_NULL_HANDLE;
    VkDeviceSize indices_offset       = 0; // 以字节为单位的偏移
    VkIndexType index_type            = VK_INDEX_TYPE_UINT16;

    union {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
    };

    void draw(const VkCommandBuffer &cb) {
        if (vertices_buffer == VK_NULL_HANDLE)
            return;
        vkCmdBindVertexBuffers(cb, 0, 1, &vertices_buffer, &vertices_offset);
        if (indices_buffer != VK_NULL_HANDLE && indexed_command.indexCount != 0) {
            vkCmdBindIndexBuffer(cb, indices_buffer, indices_offset, index_type);
            vkCmdDrawIndexed(cb, indexed_command.indexCount,
                             indexed_command.instanceCount,
                             indexed_command.firstIndex,
                             indexed_command.vertexOffset,
                             indexed_command.firstInstance);
        } else if (vertex_command.vertexCount != 0) {
            vkCmdDraw(cb, vertex_command.vertexCount,
                      vertex_command.instanceCount,
                      vertex_command.firstVertex,
                      vertex_command.firstInstance);
        }
    }
};
#endif


struct buffer_and_share {
#ifdef WITH_VULKAN_BACKEND
    Model_mesh mesh;
#elif  WITH_OPENGL_BACKEND
    unsigned int buffer;
#endif
    uint16_t shared_number;
};

struct pipeline_and_share {
#ifdef WITH_VULKAN_BACKEND
    VkPipeline pipeline = VK_NULL_HANDLE;
#elif  WITH_OPENGL_BACKEND
    unsigned int buffer;
#endif
    uint16_t shared_number = 0;
};

class logic_render_data;

using Shared_ptr_of_vertices = std::shared_ptr<void>;

// 一般情况下是这两种选择
// VK_INDEX_TYPE_UINT16 = 0,
// VK_INDEX_TYPE_UINT32 = 1,
using Indices_type = std::shared_ptr<std::vector<u_int16_t> >;


struct VertexAttrib {
    uint32_t size;
    unsigned int type;
    bool normalized;
    /**
     *
     * @param size 表示有几个数据
     * @param type 类型，表示其中单个数据的类型
     * @param normalized 是否需要归一化
     * @param stride 间隔，重新下一个数据需要间隔多远
     * @param pointer 访问时是否需要偏移
     */
    VertexAttrib(uint32_t size,
                 unsigned int type,
                 bool normalized
    ) : size(size), type(type), normalized(normalized) {
    }
};

struct vertex_and_attributes {
    Shared_ptr_of_vertices shared_ptr_of_vertices_;
    void *data;
    unsigned long size;
    std::vector<VertexAttrib> vertex_attribs;
};


class Texture_TBO {
public:
    std::string path_;
    std::string texture_name_;

    bool set_path(const std::string &path, const std::string &texture_name) {
        this->path_         = path;
        this->texture_name_ = texture_name;
        return true;
    }

    char const *get_texture_name() const {
        return texture_name_.c_str();
    }
};


struct draw_need_vk {
    std::string debug_name;
    VkPipeline vk_pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<VkDescriptorSet> vk_descriptor_set;
    VkViewport viewport;
    VkRect2D scissor;
    VkDeviceAddress push_constants_address;
    std::optional<float> line_width;
    Model_mesh mesh;
};


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
    uint32_t buffer_count;
    VkBuffer buffer[16];
    VkDeviceSize offset[16];

    bool operator==(const VKVertexBufferBindings &other) const {
        return buffer_count == other.buffer_count &&
               buffer[0] == other.buffer[0] &&
               buffer[1] == other.buffer[1] &&
               buffer[2] == other.buffer[2] &&
               buffer[3] == other.buffer[3] &&
               buffer[4] == other.buffer[4] &&
               buffer[5] == other.buffer[5] &&
               buffer[6] == other.buffer[6] &&
               buffer[7] == other.buffer[7] &&
               buffer[8] == other.buffer[8] &&
               buffer[9] == other.buffer[9] &&
               buffer[10] == other.buffer[10] &&
               buffer[11] == other.buffer[11] &&
               buffer[12] == other.buffer[12] &&
               buffer[13] == other.buffer[13] &&
               buffer[14] == other.buffer[14] &&
               buffer[15] == other.buffer[15] &&
               offset[0] == other.offset[0] &&
               offset[1] == other.offset[1] &&
               offset[2] == other.offset[2] &&
               offset[3] == other.offset[3] &&
               offset[4] == other.offset[4] &&
               offset[5] == other.offset[5] &&
               offset[6] == other.offset[6] &&
               offset[7] == other.offset[7] &&
               offset[8] == other.offset[8] &&
               offset[9] == other.offset[9] &&
               offset[10] == other.offset[10] &&
               offset[11] == other.offset[11] &&
               offset[12] == other.offset[12] &&
               offset[13] == other.offset[13] &&
               offset[14] == other.offset[14] &&
               offset[15] == other.offset[15];
    }

    bool operator!=(const VKVertexBufferBindings &other) const {
        return !(*this == other);
    }
};

struct VKBoundPipelines {
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


#endif //HELLO_MAC_SHADER_COMMON_H

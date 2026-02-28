//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_SHADER_COMMON_H
#define HELLO_MAC_SHADER_COMMON_H

#include <list>
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


class VKR_buffer {
    VkBuffer buffer_handle   = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    uint64_t timeline_       = 0;

public:
    VKR_buffer(const VkBuffer buffer_handle, const VmaAllocation allocation) : buffer_handle(buffer_handle),
                                                                               allocation(allocation) {
    }

    ~VKR_buffer();

    [[nodiscard]] void *mapped_address() const;

    [[nodiscard]] VkDeviceAddress get_gpu_device_address() const;

    [[nodiscard]] bool host_visible() const;

    [[nodiscard]] bool need_flush() const;

    // timeline 会和这个函数强关联
    [[nodiscard]] VkBuffer get_buffer_handle() const {
        return buffer_handle;
    }

    // timeline 会和这个函数强关联
    [[nodiscard]] const VkBuffer *get_buffer_handle_ptr() const {
        return &buffer_handle;
    }

    bool unmap_memory() const;

    bool DestroyBuffer();

    void *map_memory() const;

    bool flush(VkDeviceSize offset = 0, VkDeviceSize size = 0) const;

    [[nodiscard]] bool empty() const {
        if (buffer_handle == VK_NULL_HANDLE || allocation == VK_NULL_HANDLE) {
            return true;
        } else {
            return false;
        }
    }
};


// using VKR_buffer_ptr = std::shared_ptr<VKR_buffer>;

class VKR_buffer_ptr {
public:
    VKR_buffer_ptr(const VkBuffer buffer_handle,
                   const VmaAllocation allocation) : ptr(std::make_shared<VKR_buffer>(buffer_handle, allocation)) {
    }

    VKR_buffer_ptr() = default;

    VKR_buffer *operator->() const { return ptr.get(); }

private:
    std::shared_ptr<VKR_buffer> ptr = nullptr;
};


struct address_and_length {
    uint64_t address = 0;
    uint64_t length  = 0;
    bool if_used     = false;
};

class VKR_buffer_pool : public VKR_buffer_ptr {
public:
    VKR_buffer_pool(const VkBuffer buffer_handle, const VmaAllocation allocation) : VKR_buffer_ptr(buffer_handle,
             allocation) {
    }


    std::list<address_and_length> memory_pool;

    uint64_t alloc_size(const uint64_t size) {
        uint64_t return_address = -1;
        for (auto it = memory_pool.begin(); it != memory_pool.end(); ++it) {
            if (it->if_used == false && it->length == size) {
                it->if_used    = true;
                return_address = it->address;
                break;
            }
            if (it->if_used == false && it->length > size) {
                memory_pool.insert(it, address_and_length{it->address, size, true});
                return_address = it->address;
                it->address    += size;
                it->length     -= size;
                break;
            }
        }
        return return_address;
    }
};

void discard_buffer_map_clean();

// Vulkan 的核心目标是“零隐式开销”。
// 预计算：当你创建 VkPipeline 时，驱动程序会针对你指定的拓扑结构、顶点格式和着色器进行“整体优化编译”
// 所以拓扑结构不在这里，而在管线中
// 某些着色器阶段对拓扑结构有严格的要求
// 倾向于为不同的拓扑结构预创建不同的 Pipeline
class Model_mesh {
public:
    // 不做
    VKR_buffer_ptr vertices      = {};
    VKR_buffer_ptr indices       = {};
    VkDeviceSize vertices_offset = 0; // 以字节为单位的偏移
    VkDeviceSize indices_offset  = 0; // 以字节为单位的偏移
    VkIndexType index_type       = VK_INDEX_TYPE_UINT16;

    union {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
    };

    // 多的话上面的两个内容是需要更改为 vector 的，可能还需要 material 的指针

    void draw(const VkCommandBuffer &cb) {
        if (vertices->get_buffer_handle() == VK_NULL_HANDLE)
            return;
        vkCmdBindVertexBuffers(cb, 0, 1, vertices->get_buffer_handle_ptr(), &vertices_offset);
        if (indices->get_buffer_handle() != VK_NULL_HANDLE && indexed_command.indexCount != 0) {
            vkCmdBindIndexBuffer(cb, indices->get_buffer_handle(), indices_offset, index_type);
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


struct mesh_and_share {
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

//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_RENDER_MESH_H
#define HELLO_MAC_RENDER_MESH_H
#include <volk.h>
#include <absl/container/flat_hash_map.h>

#include "Geometry_data.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_buffer.h"


class VKR_Primitive {
public:
    uint32_t indexCount    = 0;
    uint32_t instanceCount = 0;
    uint32_t firstIndex    = 0;
    int32_t vertexOffset   = 0;
    uint32_t firstInstance = 0;
    // uint32_t material_index_ = 0;
};


class Mesh_data {
public:
    VKR_buffer_ptr vertices      = {};
    VKR_buffer_ptr indices       = {};
    VkDeviceSize vertices_offset = 0; // 以字节为单位的偏移
    VkDeviceSize indices_offset  = 0; // 以字节为单位的偏移
    VkIndexType index_type       = VK_INDEX_TYPE_UINT16;

    struct size_and_offset {
        VkDeviceSize size;
        VkDeviceSize offset;
    };

    /**
     * 这个函数需要承担什么任务?
     * 添加相应的 Geometry_data 进入,能够找到 上次 最后存放的位置
     * 并向后添加需要的内容
     * @param data
     */
    std::vector<VKR_Primitive> add_mesh_data(const Geometry_data &data) {
        auto vertices_cpu     = data.get_vertices();
        auto indices_cpu      = data.get_indices();
        VkDeviceSize vBufSize = 0;
        Mesh_data mesh_data;

        for (const auto &vertex: vertices_cpu) {
            vBufSize += vertex.total_size;
        }
        VkDeviceSize iBufSize = 0;
        for (const auto &index: indices_cpu) {
            iBufSize += index.total_size;
            if (index.single_size == 2) {
                mesh_data.index_type = VK_INDEX_TYPE_UINT16;
            } else if (index.single_size == 4) {
                mesh_data.index_type = VK_INDEX_TYPE_UINT32;
            }
        }
        mesh_data.vertices_offset = 0;
        mesh_data.indices_offset  = vBufSize;

        // 具体的复制函数
        auto mem_copy_function = [vertices_cpu,indices_cpu](void *dst) {
            auto calculation_dst = static_cast<char *>(dst);
            for (const auto &vertex: vertices_cpu) {
                memcpy(calculation_dst, vertex.data, vertex.total_size);
                calculation_dst += vertex.total_size;
            }
            for (const auto index: indices_cpu) {
                memcpy(calculation_dst, index.data, index.total_size);
                calculation_dst += index.total_size;
            }
        };

        auto &backend              = VK_backend::instance();
        const auto vertices_buffer =
                create_vertex_index_buffer(backend, vBufSize + iBufSize, mem_copy_function);
        if (!indices_cpu.empty()) {
            mesh_data.vertices = vertices_buffer;
            mesh_data.indices  = vertices_buffer;
            return {};
        } else {
            mesh_data.index_type = VK_INDEX_TYPE_MAX_ENUM;
            mesh_data.vertices   = vertices_buffer;
            return {};
        }
    }

    /**
     * 允许移除某个 Geometry_data
     * 之后呢?  重新排序全部的 VKR_Primitive ?
     * 也是由相应的  const Geometry_data &data 去生成的
     * @param data
     */
    std::vector<VKR_Primitive> remove_mesh_data(const Geometry_data &data) {
        return {};
    }

private:
    absl::flat_hash_map<share_block, size_and_offset> map_;

    // 全部搞定之后 上面的增删完成之后, 还需要给出对应的  std::vector<VKR_Primitive> 作为返回值
};

struct Draw_command {
    union {
        VkDrawIndexedIndirectCommand indexed_command = {};
        VkDrawIndirectCommand vertex_command;
        // 想实现 一个 绘制的命令，代价似乎有点大
        // 怎么大呢？ firstInstance  instanceCount
        // 只有这两个参数
        // 那么 意味 着什么呢？AABB  model pbr_material vertices indices 等等都需要排列 到一个 buffer 上
        // 其实问题不只是 排列到 各自到 buffer 上，而是需要 按照顺序来排列
        //
    };
};


#endif //HELLO_MAC_RENDER_MESH_H

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

    Geometry_data total_data_; // 汇总全部
    Geometry_data add_dirty_;  // 汇总单帧中新增加是
    bool dirty_tag = false;    // 是否需要更新


    [[nodiscard]] bool is_dirty() const {
        return dirty_tag;
    }

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
        total_data_.add_geometry(data);
        add_dirty_.add_geometry(data);
        dirty_tag = true;
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

    [[nodiscard]] bool check_need_reserve() const {
        const auto vertices_cpu     = total_data_.get_vertices();
        const auto indices_cpu      = total_data_.get_indices();
        const VkDeviceSize vBufSize = get_size(vertices_cpu);
        const VkDeviceSize iBufSize = get_size(indices_cpu);
        if (vertices != nullptr && vertices->get_size() > vBufSize) {
            return true;
        }
        if (indices != nullptr && indices->get_size() > iBufSize) {
            return true;
        }
        return false;
    }

    void update_mesh_data() {
        auto vertices_cpu           = total_data_.get_vertices();
        auto indices_cpu            = total_data_.get_indices();
        const VkDeviceSize vBufSize = get_size(vertices_cpu);
        const VkDeviceSize iBufSize = get_size(indices_cpu);

        // 具体的复制函数
        auto mem_copy_function = [vertices_cpu](void *dst) {
            auto calculation_dst = static_cast<char *>(dst);
            for (const auto &vertex: vertices_cpu) {
                memcpy(calculation_dst, vertex.data, vertex.total_size);
                calculation_dst += vertex.total_size;
            }
        };
        auto mem_copy_function_indices = [indices_cpu](void *dst) {
            auto calculation_dst = static_cast<char *>(dst);
            for (const auto &index: indices_cpu) {
                memcpy(calculation_dst, index.data, index.total_size);
                calculation_dst += index.total_size;
            }
        };
        auto result   = append_data(vertices, 0, mem_copy_function, vBufSize);
        auto result_1 = append_data(indices, 0, mem_copy_function_indices, iBufSize);

        // 这三个算是强制内容吧
        index_type      = VK_INDEX_TYPE_UINT32;
        vertices_offset = 0;
        indices_offset  = 0;
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

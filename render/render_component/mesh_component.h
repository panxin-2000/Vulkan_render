//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_MESH_COMPONENT_H
#define HELLO_MAC_MESH_COMPONENT_H
#include "global_singleton.h"
#include "../render_common/render_mesh.h"
#include "../render_common/Geometry_data.h"


class Mesh_data {
public:
    VKR_buffer_ptr vertices_      = {};
    VKR_buffer_ptr indices_       = {};
    VkDeviceSize vertices_offset_ = 0; // 以字节为单位的偏移
    VkDeviceSize indices_offset_  = 0; // 以字节为单位的偏移
    VkIndexType index_type_       = VK_INDEX_TYPE_UINT16;

    Geometry_data total_data_; // 汇总全部
    Geometry_data add_dirty_;  // 汇总单帧中新增加是
    bool dirty_tag_ = false;   // 是否需要更新


    [[nodiscard]] bool is_dirty() const {
        return dirty_tag_;
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
        dirty_tag_ = true;
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
        if (vertices_ != nullptr && vertices_->get_size() > vBufSize) {
            return true;
        }
        if (indices_ != nullptr && indices_->get_size() > iBufSize) {
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
        auto result   = append_data(vertices_, 0, mem_copy_function, vBufSize);
        auto result_1 = append_data(indices_, 0, mem_copy_function_indices, iBufSize);

        // 这三个算是强制内容吧
        index_type_      = VK_INDEX_TYPE_UINT32;
        vertices_offset_ = 0;
        indices_offset_  = 0;
    }

    bool create_mesh_data(const Geometry_data &data);

private:
    absl::flat_hash_map<share_block, size_and_offset> map_;

    // 全部搞定之后 上面的增删完成之后, 还需要给出对应的  std::vector<VKR_Primitive> 作为返回值
};


/**
 * 创建一个mesh,所有需要的数据都在 entity 的 Geometry_data 中
 * @param entity
 * @return
 */
Mesh_data get_VKR_mesh(entt::entity entity);

std::vector<VKR_Primitive> create_primitives(entt::entity entity);

std::vector<VKR_Primitive> create_primitives(const Geometry_data &data);

#endif //HELLO_MAC_MESH_COMPONENT_H

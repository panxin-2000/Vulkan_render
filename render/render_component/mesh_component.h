//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_MESH_COMPONENT_H
#define HELLO_MAC_MESH_COMPONENT_H
#include "global_singleton.h"
#include "primitive_component.h"
#include "Geometry_data.h"


/**
 *
 */
class Mesh_data {
public:
    VKR_buffer_ptr vertices_      = {};
    VKR_buffer_ptr indices_       = {};
    VkDeviceSize vertices_offset_ = 0; // 以字节为单位的偏移
    VkDeviceSize indices_offset_  = 0; // 以字节为单位的偏移
    VkIndexType index_type_       = VK_INDEX_TYPE_UINT16;


    [[nodiscard]] bool write_bind_command(const VkCommandBuffer &cb, const uint64_t timeline) const {
        if (vertices_ != nullptr && vertices_->get_buffer_handle() != VK_NULL_HANDLE) {
            vkCmdBindVertexBuffers(cb, 0, 1,
                                   vertices_->get_buffer_handle_ptr(timeline),
                                   &vertices_offset_);
            if (indices_ != nullptr && indices_->get_buffer_handle() != VK_NULL_HANDLE) {
                vkCmdBindIndexBuffer(cb,
                                     indices_->get_buffer_handle(timeline),
                                     indices_offset_,
                                     index_type_);
            }
            return true;
        } else if (index_type_ == VK_INDEX_TYPE_MAX_ENUM) {
            assert(false && " not deal indices empty");
        }
        return false;
    }


    bool create_mesh_data(const Geometry_data &data);
};


/**
 * 创建一个mesh,所有需要的数据都在 entity 的 Geometry_data 中
 * @param entity
 * @return
 */
Mesh_data get_VKR_mesh(entt::entity entity);


#endif //HELLO_MAC_MESH_COMPONENT_H

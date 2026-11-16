//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"


bool Mesh_data::create_mesh_data(const Geometry_data &data) {
    VkDeviceSize vBufSize                    = 0;
    const std::vector<share_block> &vertices = data.get_vertices();
    const std::vector<share_block> &indices  = data.get_indices();

    for (const auto vertex: vertices) {
        vBufSize += vertex.total_size;
    }
    VkDeviceSize iBufSize = 0;
    for (const auto index: indices) {
        iBufSize += index.total_size;
        if (index.single_size == 2) {
            index_type_ = VK_INDEX_TYPE_UINT16;
        } else if (index.single_size == 4) {
            index_type_ = VK_INDEX_TYPE_UINT32;
        }
    }
    vertices_offset_ = 0;
    indices_offset_  = vBufSize;

    // 具体的复制函数
    auto mem_copy_function = [vertices,indices](void *dst) {
        auto calculation_dst = static_cast<char *>(dst);
        for (const auto vertex: vertices) {
            memcpy(calculation_dst, vertex.data, vertex.total_size);
            calculation_dst += vertex.total_size;
        }
        for (const auto index: indices) {
            memcpy(calculation_dst, index.data, index.total_size);
            calculation_dst += index.total_size;
        }
    };

    const auto vertices_buffer =
            create_vertex_index_buffer(VK_backend::instance(), vBufSize + iBufSize, mem_copy_function);
    if (!indices.empty()) {
        vertices_ = vertices_buffer;
        indices_  = vertices_buffer;
        return true;
    } else {
        index_type_ = VK_INDEX_TYPE_MAX_ENUM;
        vertices_   = vertices_buffer;
        return true;
    }
    return false;
}


Mesh_data get_VKR_mesh(const entt::entity entity) {
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        auto &temp = Logic_entt().emplace_or_replace<Mesh_data>(entity);
        temp.create_mesh_data(*data);
        return temp;
    }
    return {};
}

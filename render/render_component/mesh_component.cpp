//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"


std::vector<VKR_Primitive> create_primitives(const Geometry_data &data) {
    std::vector<VKR_Primitive> primitives;
    const std::vector<share_block> &vertices = data.get_vertices();
    const std::vector<share_block> &indices  = data.get_indices();
    if (!indices.empty() && indices.size() == vertices.size()) {
        size_t vertices_offset = 0;
        uint32_t first_index   = 0;
        for (uint32_t i = 0; i < indices.size(); i++) {
            const auto index  = indices[i];
            const auto vertex = vertices[i];
            VKR_Primitive primitive;
            primitive.vertexOffset = static_cast<int32_t>(vertices_offset); // 应该是这里的问题
            vertices_offset        += vertex.count;
            primitive.firstIndex   = first_index;
            first_index            += index.count;

            // 这里应该是没有什么问题的
            primitive.indexCount = index.count;
            //确实是可以通过计算偏移的
            primitive.instanceCount = 1; // 也就是这两个是需要去手动进行计算的
            primitive.firstInstance = 0; // 这里主要是为了进行bindless 相关的填充
            primitives.push_back(primitive);
        }
    } else {
        assert(false && " not deal indices empty");
    }
    for (uint32_t i = 0; i < primitives.size(); ++i) {
        primitives.at(i).firstInstance = i;
    }
    return primitives;
}

bool Mesh_data::create_mesh_data(const Geometry_data &data) { {
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
    }
    return false;
}


std::vector<VKR_Primitive> create_primitives(const entt::entity entity) {
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        return create_primitives(*data);
    }
    return {};
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

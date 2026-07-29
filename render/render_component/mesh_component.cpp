//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"

#include "Rect_2D_component.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"


Mesh_data create_mesh_data(const VK_backend &backend,
                           const std::vector<share_block> &vertices,
                           const std::vector<share_block> &indices) {
    VkDeviceSize vBufSize = 0;
    for (const auto vertex: vertices) {
        vBufSize += vertex.total_size;
    }
    VkDeviceSize iBufSize = 0;
    for (const auto index: indices) {
        iBufSize += index.total_size;
    }
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
            create_vertex_index_buffer(backend, vBufSize + iBufSize, mem_copy_function);
    // vertices_buffer 还需要动，firstIndex 在之后也是需要更改的
    if (!indices.empty())
        return Mesh_data{vertices_buffer, vertices_buffer};
    else {
        return Mesh_data{vertices_buffer,};
    }
}


std::vector<VKR_Primitive> create_primitives(const Geometry_data &data) {
    std::vector<VKR_Primitive> primitives;
    const std::vector<share_block> &vertices = data.get_vertices();
    const std::vector<share_block> &indices  = data.get_indices();
    VkDeviceSize vBufSize                    = 0;
    for (const auto vertex: vertices) {
        vBufSize += vertex.total_size;
    }
    if (!indices.empty() && indices.size() == vertices.size()) {
        auto total_single_size       = 0;
        int vector_index             = 0;
        VkDeviceSize vertices_offset = 0;
        uint32_t first_index         = 0;
        for (const auto index: indices) {
            VKR_Primitive primitive;
            total_single_size                                   += index.single_size;
            primitive.vertices_offset                           = 0;
            primitive.indices_offset                            = vBufSize;
            primitive.draw_command.indexed_command.vertexOffset = vertices_offset;
            // 当你使用 vkCmdBindIndexBuffer 绑定索引数据时，传入的 offset（偏移量）必须是该索引类型大小的整数倍。
            // 如果使用 uint32 索引，offset 必须能被 4 整除。如果使用 uint16 索引，offset 必须能被 2 整除。
            primitive.draw_command.indexed_command.indexCount = index.count; // 是可以这么替换的
            if (index.single_size == 2) {
                primitive.index_type                              = VK_INDEX_TYPE_UINT16;
                primitive.draw_command.indexed_command.firstIndex = first_index;
                //  // 索引缓冲区的起始偏移（以索引 VK_INDEX_TYPE_UINT16 或 VK_INDEX_TYPE_UINT32  为单位）
            } else if (index.single_size == 4) {
                primitive.index_type                              = VK_INDEX_TYPE_UINT32;
                primitive.draw_command.indexed_command.firstIndex = first_index;
            } else {
                // assert(false && "Unknown index type");
            }
            first_index += index.count;
            //确实是可以通过计算偏移的
            // primitive.draw_command.indexed_command.vertexOffset  = vertices_offset;
            primitive.draw_command.indexed_command.instanceCount = 1; // 也就是这两个是需要去手动进行计算的
            primitive.draw_command.indexed_command.firstInstance = 0; // 这里主要是为了进行bindless 相关的填充
            primitives.push_back(primitive);
        }
    } else {
        VkDeviceSize single_BufSize = 0;
        for (const auto vertex: vertices) {
            VKR_Primitive primitive;
            primitive.vertices_offset                           = single_BufSize;
            single_BufSize                                      += vertex.total_size;
            primitive.indices_offset                            = 0;
            primitive.index_type                                = VK_INDEX_TYPE_MAX_ENUM;
            primitive.draw_command.vertex_command.firstInstance = 0;
            primitive.draw_command.vertex_command.firstVertex   = 0; // 这里无用，上面的偏移 vertices_offset 起作用
            primitive.draw_command.vertex_command.instanceCount = 1;
            primitive.draw_command.vertex_command.vertexCount   = vertex.count;
            primitives.push_back(primitive);
        }
    }
    // assert(total_single_size/indices.size() == indices.at(0).single_size);
    // 不知道上面这个检查有没有用
    return primitives;
}

std::vector<VKR_Primitive> create_primitives(const entt::entity entity) {
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        return create_primitives(*data);
    }
    return {};
}

Mesh_data create_mesh_data(const Geometry_data &data) {
    return create_mesh_data(VK_backend::instance(), data.get_vertices(), data.get_indices());
}

Mesh_data get_VKR_mesh(const entt::entity entity) {
    const auto &backend = VK_backend::instance();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        const auto &values = data->get_vertices();
        auto bound_box     = find_min_max_point(values);
        auto &AABB         = Logic_entt().get_or_emplace<AABB_min_max<Point_3> >(entity, bound_box);
        return create_mesh_data(*data);
    }
    return {};
}

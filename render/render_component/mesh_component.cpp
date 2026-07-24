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


std::vector<VKR_Primitive> create_primitives(const entt::entity entity) {
    std::vector<VKR_Primitive> primitives;
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的，现在应该没有问题了
        const std::vector<share_block> &vertices = data->get_vertices();
        const std::vector<share_block> &indices  = data->get_indices();
        VkDeviceSize vBufSize                    = 0;
        for (const auto vertex: vertices) {
            vBufSize += vertex.total_size;
        }
        if (!indices.empty()) {
            auto total_single_size = 0;
            for (const auto index: indices) {
                VKR_Primitive primitive;
                total_single_size         += index.single_size;
                primitive.vertices_offset = 0; // 这里似乎不是很对？ 感觉不太对
                primitive.indices_offset  = vBufSize;
                vBufSize                  += index.total_size;
                // mesh.indices_offset = vBufSize;
                // 当你使用 vkCmdBindIndexBuffer 绑定索引数据时，传入的 offset（偏移量）必须是该索引类型大小的整数倍。
                // 如果使用 uint32 索引，offset 必须能被 4 整除。如果使用 uint16 索引，offset 必须能被 2 整除。
                primitive.indexed_command.indexCount = index.count; // 是可以这么替换的
                if (index.single_size == 2) {
                    primitive.index_type = VK_INDEX_TYPE_UINT16;
                    // primitive.indexed_command.firstIndex = vBufSize / 2;
                    //  // 索引缓冲区的起始偏移（以索引 VK_INDEX_TYPE_UINT16 或 VK_INDEX_TYPE_UINT32  为单位）
                } else if (index.single_size == 4) {
                    primitive.index_type = VK_INDEX_TYPE_UINT32;
                    // primitive.indexed_command.firstIndex = vBufSize / 4;
                } else {
                    // assert(false && "Unknown index type");
                }
                //确实是可以通过计算偏移的
                primitive.indexed_command.vertexOffset  = 0;
                primitive.indexed_command.instanceCount = 1;
                primitive.indexed_command.firstInstance = 0;
                primitives.push_back(primitive);
            }
        } else {
            VkDeviceSize single_BufSize = 0;
            for (const auto vertex: vertices) {
                VKR_Primitive primitive;
                primitive.vertices_offset              = single_BufSize;
                single_BufSize                         += vertex.total_size;
                primitive.indices_offset               = 0;
                primitive.index_type                   = VK_INDEX_TYPE_MAX_ENUM;
                primitive.vertex_command.firstInstance = 0;
                primitive.vertex_command.firstVertex   = 0; // 这里无用，上面的偏移 vertices_offset 起作用
                primitive.vertex_command.instanceCount = 1;
                primitive.vertex_command.vertexCount   = vertex.count;
                primitives.push_back(primitive);
            }
        }
        // assert(total_single_size/indices.size() == indices.at(0).single_size);
        // 不知道上面这个检查有没有用
    }
    return primitives;
}

Mesh_data get_VKR_mesh(const entt::entity entity) {
    const auto &backend = VK_backend::instance();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        const auto &values = data->get_vertices();
        auto bound_box     = find_min_max_point(values);
        auto &AABB         = Logic_entt().get_or_emplace<AABB_min_max<Point_3> >(entity, bound_box);
        return create_mesh_data(backend, data->get_vertices(), data->get_indices());
    }
    return {};
}

//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"

#include "Rect_2D_component.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"


Mesh_data create_mesh_data(const VK_backend &backend,
                           const share_block &vertices,
                           const share_block &indices) {
    VkDeviceSize vBufSize{vertices.total_size};
    VkDeviceSize iBufSize{indices.total_size};

    // 具体的复制函数
    auto mem_copy_function = [vertices,vBufSize,indices,iBufSize](void *dst) {
        memcpy(dst, vertices.data, vBufSize);
        memcpy(static_cast<char *>(dst) + vBufSize, indices.data, iBufSize);
    };

    const auto vertices_buffer =
            create_vertex_index_buffer(backend, vBufSize + iBufSize, mem_copy_function);

    // vertices_buffer 还需要动，firstIndex 在之后也是需要更改的

    Mesh_data mesh_data{vertices_buffer, vertices_buffer};
    // 下面这样做更容易被理解
    return mesh_data;
}


std::vector<VKR_Primitive> create_primitives(const entt::entity entity) {
    std::vector<VKR_Primitive> primitives;
    const auto &backend = VK_backend::instance();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        const share_block &vertices = data->get_vertices();
        const share_block &indices  = data->get_indices();
        const VkDeviceSize vBufSize{vertices.total_size};
        VKR_Primitive primitive;
        primitive.scissor  = VK_backend::instance().get_scissor();
        primitive.viewport = VK_backend::instance().get_viewport();

        primitive.vertices_offset = 0;
        primitive.indices_offset  = vBufSize;

        // mesh.indices_offset = vBufSize;
        // 当你使用 vkCmdBindIndexBuffer 绑定索引数据时，传入的 offset（偏移量）必须是该索引类型大小的整数倍。
        // 如果使用 uint32 索引，offset 必须能被 4 整除。如果使用 uint16 索引，offset 必须能被 2 整除。
        primitive.indexed_command.indexCount = indices.count; // 是可以这么替换的
        // if (indices.single_size == 2) {
        // primitive.index_type                 = VK_INDEX_TYPE_UINT16;
        // primitive.indexed_command.firstIndex = vBufSize / 2;
        //  // 索引缓冲区的起始偏移（以索引 VK_INDEX_TYPE_UINT16 或 VK_INDEX_TYPE_UINT32  为单位）
        // } else if (indices.single_size == 4) {
        // primitive.index_type                 = VK_INDEX_TYPE_UINT32;
        // primitive.indexed_command.firstIndex = vBufSize / 4;
        // } else {
        // assert(false && "Unknown index type");
        // }
        //确实是可以通过计算偏移的
        primitive.indexed_command.vertexOffset  = 0;
        primitive.indexed_command.instanceCount = 1;
        primitive.indexed_command.firstInstance = 0;
        primitives.push_back(primitive);
    }
    return primitives;
}

Mesh_data get_VKR_mesh(const entt::entity entity) {
    const auto &backend = VK_backend::instance();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        return create_mesh_data(backend, data->get_vertices(), data->get_indices());
    }
    return {};
}

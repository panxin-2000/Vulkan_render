//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"

#include "Rect_2D_component.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"


VKR_Primitive create_mesh_data(const VK_backend &backend, const share_block &vertices,
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

    VKR_Primitive mesh;
    mesh.vertices = vertices_buffer;
    mesh.indices  = vertices_buffer;

    // mesh.indices_offset = vBufSize;
    // 当你使用 vkCmdBindIndexBuffer 绑定索引数据时，传入的 offset（偏移量）必须是该索引类型大小的整数倍。
    // 如果使用 uint32 索引，offset 必须能被 4 整除。如果使用 uint16 索引，offset 必须能被 2 整除。
    mesh.indexed_command.indexCount = indices.count; // 是可以这么替换的
    if (indices.single_size == 2) {
        mesh.index_type                 = VK_INDEX_TYPE_UINT16;
        mesh.indexed_command.firstIndex = vBufSize / 2;
        // 索引缓冲区的起始偏移（以索引 VK_INDEX_TYPE_UINT16 或 VK_INDEX_TYPE_UINT32  为单位）
    } else if (indices.single_size == 4) {
        mesh.index_type                 = VK_INDEX_TYPE_UINT32;
        mesh.indexed_command.firstIndex = vBufSize / 4;
    } else {
        assert(false && "Unknown index type");
    }
    //确实是可以通过计算偏移的
    mesh.indexed_command.vertexOffset  = 0;
    mesh.indexed_command.instanceCount = 1;
    mesh.indexed_command.firstInstance = 0;


    return mesh;
}


std::vector<VKR_Primitive> create_mesh(const entt::entity entity) {
    const auto &backend = VK_backend::instance();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        const auto mesh = create_mesh_data(backend, data->get_vertices(), data->get_indices());
        std::vector<VKR_Primitive> result;
        result.push_back(mesh);
        return result;
    }
    return {};
}


std::vector<VKR_Primitive> get_VKR_mesh(const entt::entity entity) {
    const auto mesh = create_mesh(entity);
    return mesh;
}

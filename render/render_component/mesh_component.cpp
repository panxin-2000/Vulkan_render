//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"

#include "Rect_2D_component.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_backend.h"


// #define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#include "vulkan_render_manage.h"

std::map<std::string, mesh_and_share> mesh_map_;

auto &get_mesh_map() {
    return mesh_map_;
}

VKR_Primitive create_mesh_data(const VK_backend &backend, const share_block &vertices,
                               const share_block &indices_) {
    VkDeviceSize vBufSize{vertices.total_size};
    VkDeviceSize iBufSize{indices_.total_size};

    // 具体的复制函数
    auto mem_copy_function = [vertices,vBufSize,indices_,iBufSize](void *dst) {
        memcpy(dst, vertices.data, vBufSize);
        memcpy(static_cast<char *>(dst) + vBufSize, indices_.data, iBufSize);
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
    mesh.indexed_command.indexCount = indices_.count; // 是可以这么替换的
    mesh.indexed_command.firstIndex = vBufSize / 2; // 索引缓冲区的起始偏移（以索引 VK_INDEX_TYPE_UINT16 或 VK_INDEX_TYPE_UINT32  为单位）
    //确实是可以通过计算偏移的
    mesh.indexed_command.vertexOffset  = 0;
    mesh.indexed_command.instanceCount = 1;
    mesh.indexed_command.firstInstance = 0;


    return mesh;
}


std::vector<VKR_Primitive> create_mesh(const entt::entity entity) {
    const auto &backend = VK_backend::get();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        // todo : 这里的逻辑还是有问题的
        const auto mesh = create_mesh_data(backend, data->get_vertices(), data->get_indices());
        std::vector<VKR_Primitive> result;
        result.push_back(mesh);
        return result;
    }
    return {};
}


void clean_all_mesh_object() {
    // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    get_mesh_map().clear();
    discard_buffer_map_clean();
}

#include <tiny_obj_loader.h>


inline bool load_model_to_vector(const std::string &path, std::shared_ptr<std::vector<Vertex> > &vertices,
                                 std::shared_ptr<std::vector<uint16_t> > &indices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, nullptr, path.c_str());
    // todo : result need check
    if (result == false) {
        return false;
    }
    const VkDeviceSize indexCount{shapes[0].mesh.indices.size()};
    // Load vertex and index data
    for (auto &index: shapes[0].mesh.indices) {
        Vertex v{
            .pos = {
                attrib.vertices[index.vertex_index * 3], -attrib.vertices[index.vertex_index * 3 + 1],
                attrib.vertices[index.vertex_index * 3 + 2]
            },
            .normal = {
                attrib.normals[index.normal_index * 3], -attrib.normals[index.normal_index * 3 + 1],
                attrib.normals[index.normal_index * 3 + 2]
            },
            .uv = {attrib.texcoords[index.texcoord_index * 2], 1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]}
        };
        vertices->push_back(v);
        indices->push_back(indices->size());
    }
}


std::pair<const std::shared_ptr<std::vector<Vertex> >,
          const std::shared_ptr<std::vector<uint16_t> >> load_model(const std::string &path) {
    auto sp_vertices               = std::make_shared<std::vector<Vertex> >();
    auto sp_indices                = std::make_shared<std::vector<uint16_t> >();
    std::filesystem::path filePath = path;
    std::string ext                = filePath.extension().string();

    if (ext == ".obj") {
        load_model_to_vector(path, sp_vertices, sp_indices);
        return {sp_vertices, sp_indices};
    } else {
    }
    return {sp_vertices, sp_indices};
}

VkPrimitiveTopology get_primitive_topology(const tinygltf::Primitive &primitive) {
    switch (primitive.mode) {
        case TINYGLTF_MODE_POINTS:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case TINYGLTF_MODE_LINE:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case TINYGLTF_MODE_LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case TINYGLTF_MODE_TRIANGLES:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case TINYGLTF_MODE_TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default:
            // 默认的值有点问题
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
    //     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
    //     VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 6,
    //     VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 7,
    //     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 8,
    //     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 9,
    //     VK_PRIMITIVE_TOPOLOGY_PATCH_LIST = 10,
}








std::vector<VKR_Primitive> get_VKR_mesh(const entt::entity entity) {
    const auto mesh = create_mesh(entity);
    return mesh;
}


void update_object_mesh() {
    const auto view = Logic_entt().view<UI_transform_dirty, Rect_2D_transform>();
    // 包围盒发生了更新
    for (const auto it: view) {
        auto pos = view.get<Rect_2D_transform>(it);
        add_2D_bound_box_geometry(it, {
                                      pos.get_bounding_box().min_point_.x,
                                      pos.get_bounding_box().min_point_.y, 0.0f
                                  },
                                  {
                                      pos.get_bounding_box().max_point_.x,
                                      pos.get_bounding_box().max_point_.y, 0.0f
                                  });

        const auto mesh = create_mesh(it);

        logic_update_proxy(it, mesh);

        Logic_entt().remove<UI_transform_dirty>(it);
    }
}

//
// Created by 潘鑫 on 2026/3/3.
//

#include "mesh_component.h"

#include "vertex_and_buffer_index.h"
#include "vulkan_device_handle.h"

std::map<Geometry_data *, mesh_and_share> mesh_map_;

auto &get_mesh_map() {
    return mesh_map_;
}

VKR_mesh create_mesh_data(const VK_handle &handle, const share_block &vertices,
                          const share_block &indices_) {
    VkDeviceSize vBufSize{vertices.total_size};
    VkDeviceSize iBufSize{indices_.total_size};

    // 具体的复制函数
    auto mem_copy_function = [vertices,vBufSize,indices_,iBufSize](void *dst) {
        memcpy(dst, vertices.data, vBufSize);
        memcpy(static_cast<char *>(dst) + vBufSize, indices_.data, iBufSize);
    };

    const auto vertices_buffer =
            create_vertex_index_buffer(handle, vBufSize + iBufSize, mem_copy_function);

    // vertices_buffer 还需要动，firstIndex 在之后也是需要更改的

    VKR_mesh mesh;
    mesh.vertices = vertices_buffer;
    mesh.indices  = vertices_buffer;
    // mesh.indices_offset = vBufSize;
    mesh.indexed_command.indexCount = indices_.count; // 是可以这么替换的
    mesh.indexed_command.firstIndex = vBufSize / 2; // 索引缓冲区的起始偏移（以索引 VK_INDEX_TYPE_UINT16 或 VK_INDEX_TYPE_UINT32  为单位）
    //确实是可以通过计算偏移的
    mesh.indexed_command.vertexOffset  = 0;
    mesh.indexed_command.instanceCount = 1;
    mesh.indexed_command.firstInstance = 0;


    return mesh;
}


std::optional<VKR_mesh> create_mesh(const entt::entity entity) {
    std::map<Geometry_data *, mesh_and_share> &map = get_mesh_map();
    const auto &handle                             = VK_handle::get();
    if (const auto data = g_entt().try_get<Geometry_data>(entity)) {
        auto it = map.find(data);
        if (it != map.end()) {
            it->second.shared_number++;
            return it->second.mesh;
        } else {
            if (data->mesh_path_.empty() == false) {
                auto [vertices, indices] = load_model(data->mesh_path_);
                const auto mesh          = create_mesh_data(handle, vertices, indices);
                // map.insert({data, {mesh, 1}});
                return mesh;
            } else {
                for (const auto &temp: data->vertices_vector) {
                    // create_vertex_buffer(temp.shared_ptr_of_vertices_, temp.size, temp.data, &vertices_map_);
                    auto mesh = create_mesh_data(handle, temp, data->indices_);
                    // map.insert({data, {mesh, 1}});
                    return mesh;
                }
            }
        }
    }
    return {};
}


inline VKR_mesh *find_mesh(Geometry_data data,
                           std::map<Geometry_data *, mesh_and_share> &map) {
    auto it = map.find(&data);
    if (it != map.end()) {
        return &it->second.mesh;
    } else {
        return nullptr;
    }
}

void clean_all_mesh_object() {
    // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    for (const auto &[key, value]: get_mesh_map()) {
        value.mesh.vertices->destroy_buffer();
        // ->不清理会直接爆异常
    }
    get_mesh_map().clear();
    // discard_buffer_map_clean();
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
            .uv = {attrib.texcoords[index.texcoord_index * 2], 1.0 - attrib.texcoords[index.texcoord_index * 2 + 1]}
        };
        vertices->push_back(v);
        indices->push_back(indices->size());
    }
}

std::pair<share_block, share_block> load_model(const std::string &path) {
    auto sp_vertices = std::make_shared<std::vector<Vertex> >();
    auto sp_indices  = std::make_shared<std::vector<uint16_t> >();
    load_model_to_vector(path, sp_vertices, sp_indices);
    share_block vertices{
        sp_vertices,
        sp_vertices->data(),
        sp_vertices->size() * sizeof(Vertex),
        sp_vertices->size(),
        sizeof(Vertex)
    };
    share_block indices{
        sp_indices,
        sp_indices->data(),
        sp_indices->size() * sizeof(uint16_t),
        sp_indices->size(),
        sizeof(uint16_t)
    };
    return {vertices, indices};
}


bool add_geometry_data(entt::entity entity_,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y) {
    if (auto *pos = g_entt().try_get<Geometry_data>(entity_)) {
        g_entt().remove<Geometry_data>(entity_);
    }
    g_entt().emplace<Geometry_data>(entity_);

    auto &geometry = g_entt().get<Geometry_data>(entity_);

    /***************设置顶点与索引参数**********************/
    // std::vector<VertexAttrib> vertex_attribs;
    // vertex_attribs.emplace_back(3,GL_FLOAT,GL_FALSE);
    // vertex_attribs.emplace_back(2,GL_FLOAT,GL_FALSE);
    // vertex_attribs.emplace_back(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) (3 * sizeof(float)));

    struct pos_normal_uv {
        float x, y, z, a, b, c, u, v;
    };

    const auto vertices = std::make_shared<std::vector<pos_normal_uv> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();      //  2   * 6 = 12
    // 要改这里，需要改的内容似乎就有点说了，之后再看看怎么改吧。
    {
        indices->push_back(vertices->size() + 0);
        indices->push_back(vertices->size() + 1);
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 0);
        vertices->emplace_back(pos_normal_uv{min_x, min_y, 0, 0, 0, 0, 0, 0}); //0 1 2
        vertices->emplace_back(pos_normal_uv{max_x, min_y, 0, 0, 0, 0, 1, 0});
        vertices->emplace_back(pos_normal_uv{max_x, max_y, 0, 0, 0, 0, 1, 1}); // 2 3 0
        vertices->emplace_back(pos_normal_uv{min_x, max_y, 0, 0, 0, 0, 0, 1});
    }
    // 参数这里最重要的是下面的两行

    // 参数这里最重要的是下面的两行
    const share_block vertices_buffer = {
        vertices,
        vertices->data(),
        vertices->size() * sizeof(pos_normal_uv),
        vertices->size(),
        sizeof(pos_normal_uv)
    };
    const share_block indices_buffer = {
        indices,
        indices->data(),
        indices->size() * sizeof(uint16_t),
        indices->size(),
        sizeof(uint16_t)
    };

    geometry.push_vertices(vertices_buffer);
    geometry.set_indices(indices_buffer);
}

VKR_mesh get_VKR_mesh(const entt::entity entity) {
    const auto mesh = create_mesh(entity);
    if (mesh.has_value()) {
        // 打印一个 entity name 读取 mesh 错误
        return mesh.value();
    }
    return {};
}

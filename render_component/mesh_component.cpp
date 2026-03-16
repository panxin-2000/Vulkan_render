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

VKR_mesh create_mesh_data(const VK_backend &backend, const share_block &vertices,
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
    std::map<std::string, mesh_and_share> &map = get_mesh_map();
    const auto &handle                         = VK_backend::get();
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        if (data->mesh_path_.empty() == false) {
            auto it = map.find(data->mesh_path_);
            if (it != map.end()) {
                it->second.shared_number++;
                return it->second.mesh;
            }
            auto [vertices, indices] = load_model(data->mesh_path_);
            auto mesh                = create_mesh_data(handle, vertices, indices);
            map.insert({data->mesh_path_, {mesh, 1}});
            return mesh;
        } else {
            for (const auto &temp: data->vertices_vector) {
                // todo : 这里的逻辑还是有问题的
                auto mesh = create_mesh_data(handle, temp, data->indices_);
                return mesh;
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

const unsigned char *get_accessor_start_address(tinygltf::Model &model, tinygltf::Accessor &current_accessor) {
    const tinygltf::BufferView &bufferView = model.bufferViews[current_accessor.bufferView];
    const tinygltf::Buffer &buffer         = model.buffers[bufferView.buffer];
    // 数据真实起始地址 = Buffer基址 + BufferView偏移 + Accessor偏移
    const unsigned char *dataPtr = &(buffer.data[bufferView.byteOffset + current_accessor.byteOffset]);
    return dataPtr;
}


int get_element_size(tinygltf::Accessor &current_accessor) {
    int numComponents = tinygltf::GetNumComponentsInType(current_accessor.type);
    // 获取组件的字节大小（如 FLOAT 返回 4）
    int componentSize = tinygltf::GetComponentSizeInBytes(current_accessor.componentType);
    // 计算单个元素的总字节数
    int elementSize = numComponents * componentSize;
    return elementSize;
}

std::pair<share_block, share_block> load_model(const std::string &path) {
    auto sp_vertices               = std::make_shared<std::vector<Vertex> >();
    auto sp_indices                = std::make_shared<std::vector<uint16_t> >();
    std::filesystem::path filePath = path;
    std::string ext                = filePath.extension().string();

    if (ext == ".obj") {
        load_model_to_vector(path, sp_vertices, sp_indices);
    } else if (ext == ".gltf" || ext == ".glb") {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err, warn;
        bool res = false;
        // 加载 glTF/glb 文件
        if (ext == ".glb") {
            res = loader.LoadBinaryFromFile(&model, &err, &warn, path);
        } else {
            res = loader.LoadASCIIFromFile(&model, &err, &warn, path);
        }
        if (!warn.empty()) {
            LOG_INFO(g_log(), "load gltf file : {} warn {}", path, warn);
        }
        if (!err.empty()) {
            LOG_INFO(g_log(), "load gltf file : {} error {}", path, err);
        }
        if (res == false) {
            return {};
        }

        for (const auto &mesh: model.meshes) {
            for (const auto &primitive: mesh.primitives) {
                // 1. 获取索引数据（如果有）
                if (primitive.indices > -1) {
                    auto current_accessor                  = model.accessors[primitive.indices]; // 复制的函数需要处理
                    const tinygltf::BufferView &bufferView = model.bufferViews[current_accessor.bufferView];
                    // 数据真实起始地址 = Buffer基址 + BufferView偏移 + Accessor偏移
                    const unsigned char *dataPtr = get_accessor_start_address(model, current_accessor);
                    int stride                   = current_accessor.ByteStride(bufferView);
                    auto data_type               = current_accessor.componentType;
                    int data_single_size         = tinygltf::GetComponentSizeInBytes(current_accessor.componentType);
                    sp_indices.get()->resize(current_accessor.count);
                    memcpy(sp_indices.get()->data(), dataPtr, bufferView.byteLength);
                    // 简单的完成了顶点数据的复制
                    // for (size_t i = 0; i < current_accessor.count; ++i) {
                    // sp_indices->push_back( )
                    // }
                }
                std::optional<tinygltf::Accessor> position_accessor;
                std::optional<tinygltf::Accessor> normal_accessor;
                std::optional<tinygltf::Accessor> texcoord_accessor;
                // 2. 获取顶点属性（如位置、法线、纹理坐标）
                {
                    auto it = primitive.attributes.find("POSITION");
                    if (it != primitive.attributes.end()) {
                        position_accessor = model.accessors[it->second];
                    }
                } {
                    auto it = primitive.attributes.find("NORMAL");
                    if (it != primitive.attributes.end()) {
                        normal_accessor = model.accessors[it->second];
                    }
                } {
                    auto it = primitive.attributes.find("TEXCOORD_0");
                    if (it != primitive.attributes.end()) {
                        texcoord_accessor = model.accessors[it->second];
                    }
                }
                if (position_accessor.has_value() && normal_accessor.has_value() && texcoord_accessor.has_value() &&
                    position_accessor.value().count == normal_accessor.value().count &&
                    position_accessor.value().count == texcoord_accessor.value().count) {
                    int numComponents = tinygltf::GetNumComponentsInType(position_accessor.value().type);
                    // 获取组件的字节大小（如 FLOAT 返回 4）
                    int componentSize = tinygltf::GetComponentSizeInBytes(position_accessor.value().componentType);
                    // 计算单个元素的总字节数
                    int elementSize = numComponents * componentSize;


                    const unsigned char *position_data_ptr =
                            get_accessor_start_address(model, position_accessor.value());
                    const unsigned char *normal_data_ptr =
                            get_accessor_start_address(model, normal_accessor.value());
                    const unsigned char *texcoord_data_ptr =
                            get_accessor_start_address(model, texcoord_accessor.value());
                    int position_element_size = get_element_size(position_accessor.value());
                    int normal_element_size   = get_element_size(normal_accessor.value());
                    int texcoord_element_size = get_element_size(texcoord_accessor.value());


                    sp_vertices.get()->resize(position_accessor.value().count);
                    unsigned char *dst_address = reinterpret_cast<unsigned char *>(sp_vertices.get()->data());
                    for (size_t i = 0; i < position_accessor.value().count; ++i) {
                        memcpy(dst_address, position_data_ptr + i * position_element_size, position_element_size);
                        dst_address += position_element_size;
                        memcpy(dst_address, normal_data_ptr + i * normal_element_size, normal_element_size);
                        dst_address += normal_element_size;
                        memcpy(dst_address, texcoord_data_ptr + i * texcoord_element_size, texcoord_element_size);
                        dst_address += texcoord_element_size;
                    }
                }
            }
        }
    }
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


bool add_geometry_data(entt::entity entity_, const std::string &mesh_path) {
    if (auto *pos = Logic_entt().try_get<Geometry_data>(entity_)) {
        Logic_entt().remove<Geometry_data>(entity_);
    }
    Logic_entt().emplace<Geometry_data>(entity_);

    auto &geometry      = Logic_entt().get<Geometry_data>(entity_);
    geometry.mesh_path_ = mesh_path;
    return true;
}

bool add_geometry_data(entt::entity entity_,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y) {
    if (auto *pos = Logic_entt().try_get<Geometry_data>(entity_)) {
        Logic_entt().remove<Geometry_data>(entity_);
    }
    Logic_entt().emplace<Geometry_data>(entity_);

    auto &geometry = Logic_entt().get<Geometry_data>(entity_);

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

bool add_geometry_data(entt::entity entity_,
                       Point_3 a,
                       Point_3 b,
                       Point_3 c) {
    if (auto *pos = Logic_entt().try_get<Geometry_data>(entity_)) {
        Logic_entt().remove<Geometry_data>(entity_);
    }
    Logic_entt().emplace<Geometry_data>(entity_);

    auto &geometry = Logic_entt().get<Geometry_data>(entity_);

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
        vertices->emplace_back(pos_normal_uv{a.x, a.y, a.z, 0, 0, 0, 0, 0}); //0 1 2
        vertices->emplace_back(pos_normal_uv{b.x, b.y, b.z, 0, 0, 0, 1, 0});
        vertices->emplace_back(pos_normal_uv{c.x, c.y, c.z, 0, 0, 0, 1, 1}); // 2 3 0
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


void update_object_mesh() {
    const auto view = Logic_entt().view<UI_transform_dirty, Rect_2D_transform>();
    // 包围盒发生了更新
    for (const auto it: view) {
        auto pos = view.get<Rect_2D_transform>(it);
        add_geometry_data(it, pos.get_bounding_box().min_point.x,
                          pos.get_bounding_box().min_point.y,
                          pos.get_bounding_box().max_point.x,
                          pos.get_bounding_box().max_point.y);

        const auto mesh = create_mesh(it);


        if (const auto render = Logic_entt().try_get<Proxy_entity>(it)) {
            auto lambda = [render, mesh]() {
                if (const auto proxy = Render_entt().try_get<VKR_object_proxy>(render->entity_))

                    if (mesh.has_value()) {
                        proxy->mesh = mesh.value();;
                    } else {
                        LOG_INFO(g_log(), "mesh empty");
                    }
            };
            vk_render_queue::instance().render_update_entt(*render, lambda);
        }

        Logic_entt().remove<UI_transform_dirty>(it);
    }
}

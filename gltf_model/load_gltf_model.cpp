//
// Created by 潘鑫 on 2026/3/16.
//

#include "load_gltf_model.h"

#include "model_transform_component.h"
#include "name_component.h"
#include "tiny_gltf.h"


std::optional<tinygltf::Model> get_gltf_model(const std::string &path) {
    std::filesystem::path filePath = path;
    std::string ext                = filePath.extension().string();

    if (ext == ".gltf" || ext == ".glb") {
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
        return model;
    }
    return {};
}

Point_3 get_offset_from_model(const tinygltf::Model &model, const int node_index) {
    Point_3 offset;
    int nodes_num = model.nodes.size();
    if (node_index > nodes_num) {
        return {};
    } else {
        auto node = model.nodes[node_index];
        if (node.translation.size() == 0) {
            return {};
        } else if (node.translation.size() == 3) {
            offset.x = node.translation[0];
            offset.y = node.translation[1];
            offset.z = node.translation[2];
            return offset;
        }
    }
    return {};
}

Eigen::Quaternionf get_rotate_from_model(const tinygltf::Model &model, const int node_index) {
    int nodes_num = model.nodes.size();
    if (node_index > nodes_num) {
        return Eigen::Quaternionf::Identity();
    } else {
        auto node = model.nodes[node_index];
        if (node.translation.size() == 0) {
            return Eigen::Quaternionf::Identity();
        } else if (node.translation.size() == 4) {
            Eigen::Quaternionf rotate{
                static_cast<float>(node.translation[3]),
                static_cast<float>(node.translation[0]),
                static_cast<float>(node.translation[1]),
                static_cast<float>(node.translation[2])
            };
            return rotate;
        }
    }
    return Eigen::Quaternionf::Identity();
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

void copy_indices_data(const std::shared_ptr<std::vector<uint16_t> > &sp_indices, tinygltf::Model &model,
                       const tinygltf::Primitive &primitive) {
    auto current_accessor = model.accessors[primitive.indices]; // 复制的函数需要处理
    auto &bufferView      = model.bufferViews[current_accessor.bufferView];
    // 数据真实起始地址 = Buffer基址 + BufferView偏移 + Accessor偏移
    const unsigned char *dataPtr = get_accessor_start_address(model, current_accessor);
    int stride                   = current_accessor.ByteStride(bufferView);
    auto data_type               = current_accessor.componentType;
    int data_single_size         = tinygltf::GetComponentSizeInBytes(current_accessor.componentType);
    if (data_single_size == stride && current_accessor.type == TINYGLTF_TYPE_SCALAR) {
        const auto sp_indices_current_size = sp_indices->size();
        sp_indices->resize(sp_indices_current_size + current_accessor.count);
        memcpy(sp_indices->data() + sp_indices_current_size, dataPtr, bufferView.byteLength);
    } else {
        // 有间隔，需要做一些其他处理
    }
}

int get_stride(const tinygltf::Model &model, const int accessor_index) {
    const auto &current_accessor = model.accessors[accessor_index]; // 复制的函数需要处理
    const auto &bufferView       = model.bufferViews[current_accessor.bufferView];
    const int stride             = current_accessor.ByteStride(bufferView);
    return stride;
}

int get_stride(const tinygltf::Model &model, const tinygltf::Accessor &current_accessor) {
    const auto &bufferView = model.bufferViews[current_accessor.bufferView];
    const int stride       = current_accessor.ByteStride(bufferView);
    return stride;
}


void copy_vertices_data(const std::shared_ptr<std::vector<Vertex> > &sp_vertices, tinygltf::Model &model,
                        const tinygltf::Primitive &primitive) {
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
        const unsigned char *position_data_ptr =
                get_accessor_start_address(model, position_accessor.value());
        const unsigned char *normal_data_ptr =
                get_accessor_start_address(model, normal_accessor.value());
        const unsigned char *texcoord_data_ptr =
                get_accessor_start_address(model, texcoord_accessor.value());
        const int position_element_size   = get_element_size(position_accessor.value());
        const int normal_element_size     = get_element_size(normal_accessor.value());
        const int texcoord_element_size   = get_element_size(texcoord_accessor.value());
        const int position_element_stride = get_stride(model, position_accessor.value());
        const int normal_element_stride   = get_stride(model, normal_accessor.value());
        const int texcoord_element_stride = get_stride(model, texcoord_accessor.value());
        const int all_elements_size       = position_element_size + normal_element_size + texcoord_element_size;
        if (position_element_stride == all_elements_size &&
            texcoord_element_stride == all_elements_size &&
            normal_element_stride == all_elements_size &&
            0 == position_accessor.value().byteOffset &&
            position_element_size == normal_accessor.value().byteOffset &&
            position_element_size + normal_element_size == texcoord_accessor.value().byteOffset) {
            LOG_INFO(g_log(), "need deal continue position normal texcoord ");
        }

        if (position_element_stride == position_element_size &&
            normal_element_stride == normal_element_size &&
            texcoord_element_stride == texcoord_element_size) {
            const auto sp_vertices_current_size = sp_vertices->size();
            sp_vertices.get()->resize(sp_vertices_current_size + position_accessor.value().count);
            unsigned char *dst_address = reinterpret_cast<unsigned char *>
                    (sp_vertices.get()->data() + sp_vertices_current_size);
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

void get_mesh_from_gltf_model(entt::entity entity_, tinygltf::Model &model, const int mesh_index) {
    auto sp_vertices = std::make_shared<std::vector<Vertex> >();
    auto sp_indices  = std::make_shared<std::vector<uint16_t> >();

    // std::vector<VKR_Primitive> // 如果可以的话，尽可能在这里搞定，之后只需要复制一下就好
    const auto mesh    = model.meshes[mesh_index];
    int indices_count  = 0;
    int vertices_count = 0;
    for (const auto &primitive: mesh.primitives) {
        // 最开始需要能够确定数量
        if (primitive.indices > -1) {
            auto current_accessor = model.accessors[primitive.indices]; // 复制的函数需要处理
            indices_count         = indices_count + current_accessor.count;
        }
        for (const auto &attribute: primitive.attributes) {
            vertices_count += attribute.second;
        }
    }
    sp_vertices->reserve(vertices_count);
    sp_indices->reserve(indices_count);

    for (const auto &primitive: mesh.primitives) {
        if (primitive.indices > -1) {
            copy_indices_data(sp_indices, model, primitive);
        }
        copy_vertices_data(sp_vertices, model, primitive);
        // 这里只是全部放到相应的位置上了，可能需要的偏移其实没有搞定
    }

    add_geometry_data(entity_, sp_vertices, sp_indices);
}

/**
 *
 * @param model
 * @param current_node_index
 * @param parent_node_index   好像确实没有什么用
 * @param parent_node_entity
 * @return
 */
entt::entity load_node_data(tinygltf::Model &model,
                            const int current_node_index,
                            const int parent_node_index           = -1,
                            const entt::entity parent_node_entity = entt::null) {
    const entt::entity entity_ = Logic_entt().create();
    auto node                  = model.nodes[current_node_index];
    Point_3 offset             = get_offset_from_model(model, current_node_index);
    Eigen::Quaternionf rotate  = get_rotate_from_model(model, current_node_index);
    Logic_entt().emplace<Name_component>(entity_, node.name);
    // add_geometry_data(entity_, path);
    Logic_entt().emplace<VKR_shader_paths>(entity_,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/multiple_render_targets.frag.spv",
                                           "", "");
    Logic_entt().emplace<model_transform>(entity_, offset, rotate);
    const auto &transform  = Logic_entt().get<model_transform>(entity_);
    const auto modelMatrix = transform.update_model_matrix();
    set_render_parameter(entity_, "model_4x4", modelMatrix);
    world_root_add_child(entity_);
    Logic_entt().emplace_or_replace<add_to_render_tag>(entity_);
    if (node.mesh >= 0) {
        // mesh 中可以有多个 Primitive, 但是其中每个 Primitive 都是必须要绘制的，而不是可选的
        get_mesh_from_gltf_model(entity_, model, node.mesh);
    }
    if (node.camera >= 0) {
        LOG_INFO(g_log(), "need deal node  camera ");
    }
    if (node.light >= 0) {
        LOG_INFO(g_log(), "need deal node  light ");
    }
    if (node.skin >= 0) {
        LOG_INFO(g_log(), "need deal node  skin ");
    }
    if (node.emitter >= 0) {
        LOG_INFO(g_log(), "need deal node  emitter ");
    }

    if (parent_node_entity == entt::null) {
        world_root_add_child(entity_);
    } else {
        add_relation(parent_node_entity, entity_);
    }
    for (int i = 0; i < node.children.size(); ++i) {
        load_node_data(model, node.children[i], current_node_index, entity_);
    }
    return entity_;
}


entt::entity load_gltf_model(const std::string &name, const std::string &path) {
    entt::entity entity_;
    auto optional_model = get_gltf_model(path);
    if (optional_model.has_value()) {
        auto &model          = optional_model.value();
        const auto nodes_num = model.nodes.size();
        if (nodes_num > 0) {
            entity_ = load_node_data(model, 0, -1, entt::null);
        } else {
            // 空的
            entity_ = entt::null;
        }
    }
    return entity_;
}

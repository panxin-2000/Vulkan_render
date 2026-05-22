//
// Created by 潘鑫 on 2026/3/16.
//

#include "load_gltf_model.h"

#include "input_component.h"
#include "model_transform_component.h"
#include "name_component.h"
#include "PBR_component.h"
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
    Point_3 offset{0, 0, 0};
    int nodes_num = model.nodes.size();
    if (node_index > nodes_num) {
        return {0, 0, 0};
    } else {
        auto node = model.nodes[node_index];
        if (node.translation.empty()) {
            return {0, 0, 0};
        } else if (node.translation.size() == 3) {
            offset.x = node.translation[0];
            offset.y = node.translation[1];
            offset.z = node.translation[2];
            return offset;
        }
    }
    return {0, 0, 0};
}

std::vector<double> get_matrix_from_model(const tinygltf::Model &model, const int node_index) {
    std::vector<double> translation;
    int nodes_num = model.nodes.size();
    if (node_index > nodes_num) {
        return translation;
    } else {
        auto node = model.nodes[node_index];
        if (node.translation.empty()) {
            return translation;
        } else if (node.translation.size() == 3) {
            return node.matrix;
        }
    }
    return translation;
}

Point_3 get_zoom_from_model(const tinygltf::Model &model, const int node_index) {
    Point_3 zoom{1, 1, 1};
    int nodes_num = model.nodes.size();
    if (node_index > nodes_num) {
        return {1, 1, 1};
    } else {
        auto node = model.nodes[node_index];
        if (node.scale.empty()) {
            return {1, 1, 1};
        } else if (node.scale.size() == 3) {
            zoom.x = node.scale[0];
            zoom.y = node.scale[1];
            zoom.z = node.scale[2];
            return zoom;
        }
    }
    return {1, 1, 1};
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

struct Attribute {
    const unsigned char *data_ptr = nullptr;
    size_t element_size           = 0;
    size_t element_count          = 0;
    size_t element_stride         = 0;
};

unsigned char *memcpy_attribute(unsigned char *dst_address, Attribute attribute, const uint32_t index) {
    if (attribute.data_ptr != nullptr)
        memcpy(dst_address, attribute.data_ptr + index * attribute.element_size, attribute.element_size);
    dst_address += attribute.element_size;
    return dst_address;
}

void read_attribute(tinygltf::Model &model, tinygltf::Accessor &accessor, Attribute &attribute) {
    attribute.data_ptr       = get_accessor_start_address(model, accessor);
    attribute.element_size   = get_element_size(accessor);
    attribute.element_stride = get_stride(model, accessor);
    attribute.element_count  = accessor.count;
}

unsigned char *memcopy_all_attributes(const std::shared_ptr<std::vector<Vertex> > &sp_vertices,
                                      const Attribute &position,
                                      const Attribute &normal,
                                      const Attribute &texcoord) {
    const uint32_t count                = position.element_count;
    const auto sp_vertices_current_size = sp_vertices->size();
    sp_vertices->resize(sp_vertices_current_size + count);
    auto dst_address = reinterpret_cast<unsigned char *>(sp_vertices->data() + sp_vertices_current_size);
    for (size_t i = 0; i < count; ++i) {
        dst_address = memcpy_attribute(dst_address, position, i);
        dst_address = memcpy_attribute(dst_address, normal, i);
        dst_address = memcpy_attribute(dst_address, texcoord, i);
    }
}


void copy_vertices_data(const std::shared_ptr<std::vector<Vertex> > &sp_vertices, tinygltf::Model &model,
                        const tinygltf::Primitive &primitive) {
    Attribute position = {nullptr, 12, 0};
    Attribute normal   = {nullptr, 12, 0};
    Attribute texcoord = {nullptr, 8, 0};

    // todo: 这三个可以看看应该怎么删除了，下一步要做的
    std::optional<tinygltf::Accessor> position_accessor;
    std::optional<tinygltf::Accessor> normal_accessor;
    std::optional<tinygltf::Accessor> texcoord_accessor;

    // 2. 获取顶点属性（如位置、法线、纹理坐标）
    {
        auto it = primitive.attributes.find("POSITION");
        if (it != primitive.attributes.end()) {
            position_accessor = model.accessors[it->second];
            read_attribute(model, model.accessors[it->second], position);
        }
    } {
        auto it = primitive.attributes.find("NORMAL");
        if (it != primitive.attributes.end()) {
            normal_accessor = model.accessors[it->second];
            read_attribute(model, model.accessors[it->second], normal);
        }
    } {
        auto it = primitive.attributes.find("TEXCOORD_0");
        if (it != primitive.attributes.end()) {
            texcoord_accessor = model.accessors[it->second];
            read_attribute(model, model.accessors[it->second], texcoord);
        }
    }
    memcopy_all_attributes(sp_vertices, position, normal, texcoord);
    // 之后就是看如何进行细分加速了
    // 这里可以直接用一个替代的原因是 position, normal, texcoord 的 count 是一致的，不一致就会有问题
    return;
    if (position_accessor.has_value() && normal_accessor.has_value() && !texcoord_accessor.has_value() &&
        position_accessor.value().count == normal_accessor.value().count) {
        if (position.element_stride == position.element_size && normal.element_stride == normal.element_size) {
            // 这里if的判断是为了确定是 三个属性是 单独 存储的
            memcopy_all_attributes(sp_vertices, position, normal, texcoord);
        }
    }
    if (position_accessor.has_value() && !normal_accessor.has_value() && !texcoord_accessor.has_value()) {
        memcopy_all_attributes(sp_vertices, position, normal, texcoord);
    }
    if (position_accessor.has_value() && !normal_accessor.has_value() && texcoord_accessor.has_value()) {
        memcopy_all_attributes(sp_vertices, position, normal, texcoord);
    }

    if (position_accessor.has_value() && normal_accessor.has_value() && texcoord_accessor.has_value() &&
        position_accessor.value().count == normal_accessor.value().count &&
        position_accessor.value().count == texcoord_accessor.value().count) {
        const int all_elements_size = position.element_size + normal.element_size + texcoord.element_size;
        if (position.element_stride == all_elements_size &&
            texcoord.element_stride == all_elements_size &&
            normal.element_stride == all_elements_size &&
            0 == position_accessor.value().byteOffset &&
            position.element_size == normal_accessor.value().byteOffset &&
            position.element_size + normal.element_size == texcoord_accessor.value().byteOffset) {
            LOG_INFO(g_log(), "need deal continue position normal texcoord ");
            assert(false && "need deal continue position normal texcoord");
        }

        if (position.element_stride == position.element_size &&
            normal.element_stride == normal.element_size &&
            texcoord.element_stride == texcoord.element_size) {
            memcopy_all_attributes(sp_vertices, position, normal, texcoord);
        }
    }
}

void get_mesh_from_gltf_model(entt::entity entity, tinygltf::Model &model, const int mesh_index) {
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
            auto current_accessor = model.accessors[attribute.second]; // 复制的函数需要处理
            vertices_count        += current_accessor.count;
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
    auto [min, max] = find_min_max_point(sp_vertices);
    auto &AABB      = Logic_entt().get_or_emplace<AABB_centroid<Point_3> >(entity, AABB_centroid<Point_3>(min, max));
    add_geometry_data(entity, sp_vertices, sp_indices);
}


void set_model_matrix(const entt::entity entity, const tinygltf::Model &model, const int current_node_index) {
    Eigen::Matrix4f temp_matrix;
    auto temp = get_matrix_from_model(model, current_node_index);
    if (!temp.empty()) {
        for (int i = 0; i < temp.size(); ++i) {
            auto *p_float = reinterpret_cast<float *>(&temp_matrix);
            p_float[i]    = static_cast<float>(temp.at(i));
        }
        const auto &transform = Logic_entt().emplace_or_replace<model_transform>(entity, temp_matrix);
        set_render_parameter(entity, "model_4x4", temp_matrix);
        return;
    }
    // matrix 与之前的内容互斥 搞定互斥的部分

    Point_3 offset            = get_offset_from_model(model, current_node_index);
    Point_3 zoom              = get_zoom_from_model(model, current_node_index);
    Eigen::Quaternionf rotate = get_rotate_from_model(model, current_node_index);
    const auto &transform     = Logic_entt().emplace_or_replace<model_transform>(entity, offset, rotate, zoom);
    const auto modelMatrix    = transform.get_transform_matrix();
    set_render_parameter(entity, "model_4x4", modelMatrix);
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
                            std::vector<bool> &nodes_have_deal,
                            const int current_node_index,
                            const int parent_node_index           = -1,
                            const entt::entity parent_node_entity = entt::null) {
    entt::entity entity                    = entt::null;
    auto node                              = model.nodes[current_node_index];
    nodes_have_deal.at(current_node_index) = true;

    entity = Logic_entt().create();

    // 改的太多，我都忘记下面一行是需要添加的了
    logic_create_proxy(entity);
    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
                                           "", "");
    auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
    set_render_parameter(entity, "object_material", material);
    Logic_entt().emplace<Name_component>(entity, node.name);
    set_model_matrix(entity, model, current_node_index);

    if (node.mesh >= 0) {
        // mesh 中可以有多个 Primitive, 但是其中每个 Primitive 都是必须要绘制的，而不是可选的
        get_mesh_from_gltf_model(entity, model, node.mesh);
        Logic_entt().emplace<Input_Component>(entity, model_3d_Event);
        world_root_add_child(entity);
        Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
        logic_update_add_tag<opacity_tag>(entity);
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
        world_root_add_child(entity);
    } else {
        add_relation(parent_node_entity, entity);
    }
    for (int i = 0; i < node.children.size(); ++i) {
        load_node_data(model, nodes_have_deal, node.children[i], current_node_index, entity);
    }
    return entity;
}


Texture_parameter load_image(tinygltf::Image &image) {
    if (image.width * image.height * image.component * image.bits / 8 == image.image.size()) {
        if (image.component == 4) {
            Picture_parameters picture_parameters{
                image.width,
                image.height,
                image.component,
                image.image.data(),
            };
            auto texture = create_2d_texture(picture_parameters);
            return texture;
            // 确定了可以直接上传 RGBA
        }
    }

    if (image.mimeType == "image/jpeg") {
    } else if (image.mimeType == "image/png") {
    } else if (image.mimeType == "image/bmp") {
    } else if (image.mimeType == "image/gif") {
    }
    return {};
}

void load_material(const entt::entity entity, tinygltf::Model &model) {
    for (const auto &material: model.materials) {
        auto pbr_material = Logic_entt().get_or_emplace<PBR_component>(entity);

        pbr_material.metallicFactor_  = material.pbrMetallicRoughness.metallicFactor;
        pbr_material.roughnessFactor_ = material.pbrMetallicRoughness.roughnessFactor;
        if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
            pbr_material.baseColorFactor_ = {
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
                static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3]),
            };
        } else {
            pbr_material.baseColorFactor_ = {1.0f, 1.0f, 1.0f, 1.0f};
        }
        if (material.emissiveFactor.size() == 3) {
            pbr_material.emissiveFactor_ = {
                static_cast<float>(material.emissiveFactor[0]),
                static_cast<float>(material.emissiveFactor[1]),
                static_cast<float>(material.emissiveFactor[2]),
                1.0f,
            };
        } else {
            pbr_material.emissiveFactor_ = {0.0f, 0.0f, 0.0f, 1.0f};
        }
        pbr_material.occlusion_strength_ = static_cast<float>(material.occlusionTexture.strength);
        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            const auto texture_index              = material.pbrMetallicRoughness.baseColorTexture.index;
            const auto image_index                = model.textures[texture_index].source;
            auto &image                           = model.images[image_index];
            auto texture                          = load_image(image);
            std::optional<Texture_parameter> temp = texture;
            uint32_t index                        = add_bindless_uniform_sampler2D("baseColor", temp);
            pbr_material.baseColorTexture         = index;
        }
        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
            const auto texture_index              = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
            const auto image_index                = model.textures[texture_index].source;
            auto &image                           = model.images[image_index];
            auto texture                          = load_image(image);
            std::optional<Texture_parameter> temp = texture;
            uint32_t index                        = add_bindless_uniform_sampler2D("metallicRoughness", temp);
            pbr_material.ORM_Texture              = index;
        }
        if (material.normalTexture.index >= 0) {
            const auto texture_index              = material.normalTexture.index;
            const auto image_index                = model.textures[texture_index].source;
            auto &image                           = model.images[image_index];
            auto texture                          = load_image(image);
            std::optional<Texture_parameter> temp = texture;
            set_render_parameter(entity, "normal", temp);
            uint32_t index             = add_bindless_uniform_sampler2D("normal", temp);
            pbr_material.normalTexture = index;
        }
        if (material.occlusionTexture.index >= 0) {
            const auto texture_index              = material.occlusionTexture.index;
            const auto image_index                = model.textures[texture_index].source;
            auto &image                           = model.images[image_index];
            auto texture                          = load_image(image);
            std::optional<Texture_parameter> temp = texture;
            uint32_t index                        = add_bindless_uniform_sampler2D("occlusion", temp);
            // pbr_material.ORM_Texture              = index;
            // todo: ORM_Texture 需要合并两张贴图 问题是在这里应该如何合并
        }
        if (material.emissiveTexture.index >= 0) {
            const auto texture_index              = material.emissiveTexture.index;
            const auto image_index                = model.textures[texture_index].source;
            auto &image                           = model.images[image_index];
            auto texture                          = load_image(image);
            std::optional<Texture_parameter> temp = texture;
            uint32_t index                        = add_bindless_uniform_sampler2D("emissive", temp);
            pbr_material.emissiveTexture          = index;
        }
        set_render_parameter(entity, "object_material", pbr_material);
    }
}


entt::entity load_gltf_model(const std::string &name, const std::string &path) {
    entt::entity entity = entt::null;
    auto optional_model = get_gltf_model(path);
    if (optional_model.has_value()) {
        auto &model          = optional_model.value();
        const auto nodes_num = model.nodes.size();
        std::vector<bool> nodes_have_deal;
        nodes_have_deal.resize(nodes_num, false);

        for (auto i = 0; i < nodes_num && nodes_have_deal.at(i) == false; ++i) {
            // 这里也稍微有点问题 一个节点在 children 数组中只能被引用一次（即每个节点只能有一个父亲）
            entity = load_node_data(model, nodes_have_deal, i, -1, entt::null);
            load_material(entity, model);
        }
    }
    return entity;
}

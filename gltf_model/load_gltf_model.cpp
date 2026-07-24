//
// Created by 潘鑫 on 2026/3/16.
//

#include "load_gltf_model.h"
#include "input_component.h"
#include "transform_component.h"
#include "name_component.h"
#include "PBR_component.h"
#include "tiny_gltf.h"
#include "3d_model_display.h"

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

auto copy_indices_data(tinygltf::Model &model, const tinygltf::Primitive &primitive) {
    auto current_accessor  = model.accessors[primitive.indices]; // 复制的函数需要处理
    const auto &bufferView = model.bufferViews[current_accessor.bufferView];
    // 数据真实起始地址 = Buffer基址 + BufferView偏移 + Accessor偏移
    const unsigned char *src   = get_accessor_start_address(model, current_accessor);
    const int stride           = current_accessor.ByteStride(bufferView);
    auto data_type             = current_accessor.componentType;
    const int data_single_size = tinygltf::GetComponentSizeInBytes(current_accessor.componentType);
    share_block result;
    // auto address       = malloc(current_accessor.count * data_single_size);
    result.ptr         = std::make_shared<char[]>(current_accessor.count * data_single_size);
    result.count       = current_accessor.count;
    result.single_size = data_single_size;
    result.total_size  = current_accessor.count * data_single_size;
    result.data        = result.ptr.get();
    if (data_single_size == stride && current_accessor.type == TINYGLTF_TYPE_SCALAR) {
        memcpy(result.data, src, result.total_size);
    } else {
        // 有间隔，需要做一些其他处理
    }
    return result;
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
    std::string name;
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

auto mem_copy_all_attributes(const uint32_t count, std::vector<Attribute> attributes) {
    share_block result;
    auto single_size = 0;
    for (auto attribute: attributes) {
        single_size += attribute.element_size;
    }
    result.ptr         = std::make_shared<char[]>(count * single_size);
    result.count       = count;
    result.single_size = single_size;
    result.total_size  = result.count * result.single_size;
    result.data        = result.ptr.get();
    auto dst_address   = static_cast<unsigned char *>(result.data);
    for (size_t i = 0; i < count; ++i) {
        for (auto attribute: attributes) {
            dst_address = memcpy_attribute(dst_address, attribute, i);
        }
    }
    return result;
}


auto copy_vertices_data(size_t size, tinygltf::Model &model, const tinygltf::Primitive &primitive) {
    Attribute position   = {"position", nullptr, 12, 0};
    Attribute normal     = {"normal", nullptr, 12, 0};
    Attribute texcoord_0 = {"texcoord_0", nullptr, 8, 0};
    Attribute joints_0   = {"joints_0", nullptr, 8, 0};
    Attribute weights_0  = {"weights_0", nullptr, 8, 0};


    // 2. 获取顶点属性（如位置、法线、纹理坐标）
    {
        auto it = primitive.attributes.find("POSITION");
        if (it != primitive.attributes.end()) {
            read_attribute(model, model.accessors[it->second], position);
        }
    } {
        auto it = primitive.attributes.find("NORMAL");
        if (it != primitive.attributes.end()) {
            read_attribute(model, model.accessors[it->second], normal);
        }
    } {
        auto it = primitive.attributes.find("TEXCOORD_0");
        if (it != primitive.attributes.end()) {
            read_attribute(model, model.accessors[it->second], texcoord_0);
        }
    }

    std::vector<Attribute> attributes;
    attributes.push_back(position);
    attributes.push_back(normal);
    attributes.push_back(texcoord_0);

    std::vector<std::string> find_strings;
    find_strings.push_back("JOINTS_0");
    find_strings.push_back("WEIGHTS_0");
    for (auto find_string: find_strings) {
        auto it = primitive.attributes.find(find_string);
        if (it != primitive.attributes.end()) {
            Attribute attribute_temp;
            attribute_temp.name = find_string;
            read_attribute(model, model.accessors[it->second], attribute_temp);
            attributes.push_back(attribute_temp);
        }
    }
    return mem_copy_all_attributes(position.element_count, attributes);
    // 上面的做法应该是 有几个类型就复制几个属性，没有就跳过
}

std::pair<PBR_component, PBR_component_ptr> load_material(tinygltf::Model &model, int material_index);

void get_material_from_gltf_model(entt::entity entity, tinygltf::Model &model, const int mesh_index) {
    const auto mesh = model.meshes[mesh_index];
    // 现在的问题的是 一个 mesh 里面有多个 primitives
    //  PBR 需要 一个 vector
    //  然后再给每个 primitive 一个单独的索引
    // 能够去 pbr 的 vector 里面找到具体的 pbr
    // 这里的问题变成了是 单独开一个呢？ 还是 global 一下，全部慢慢索引呢？
    for (const auto &primitive: mesh.primitives) {
        if (primitive.material > -1) {
            auto result = load_material(model, primitive.material);
        }
    }
}

void get_mesh_from_gltf_model(entt::entity entity, tinygltf::Model &model, const int mesh_index,
                              const int material_base_index) {
    // std::vector<VKR_Primitive> // 如果可以的话，尽可能在这里搞定，之后只需要复制一下就好
    const auto mesh          = model.meshes[mesh_index];
    int indices_memory_size  = 0;
    int vertices_memory_size = 0;
    for (const auto &primitive: mesh.primitives) {
        // 最开始需要能够确定数量
        if (primitive.indices > -1) {
            const auto current_accessor = model.accessors[primitive.indices]; // 复制的函数需要处理
            const int data_single_size  = tinygltf::GetComponentSizeInBytes(current_accessor.componentType);
            indices_memory_size         += current_accessor.count * data_single_size;
        }

        for (const auto &attribute: primitive.attributes) {
            const auto current_accessor = model.accessors[attribute.second]; // 复制的函数需要处理
            const int data_single_size  = tinygltf::GetComponentSizeInBytes(current_accessor.componentType);
            vertices_memory_size        += current_accessor.count * data_single_size;
        }
    }

    for (const auto &primitive: mesh.primitives) {
        if (primitive.indices > -1) {
            auto result = copy_indices_data(model, primitive);
            Logic_entt().get_or_emplace<Geometry_data>(entity).push_indices(result);
        }
        auto result = copy_vertices_data(vertices_memory_size, model, primitive);
        Logic_entt().get_or_emplace<Geometry_data>(entity).push_vertices(result);
    }


    // auto bound_box = find_min_max_point(sp_vertices);
    // auto &AABB     = Logic_entt().get_or_emplace<AABB_min_max<Point_3> >(entity, bound_box);


    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity)); //  这里还是能改一些内容的
    logic_update_add_tag<opacity_tag>(entity);
}


void set_model_matrix(const entt::entity entity, const tinygltf::Model &model, const int current_node_index) {
    Eigen::Matrix4f temp_matrix = Eigen::Matrix4f::Identity();
    auto temp                   = get_matrix_from_model(model, current_node_index);
    if (!temp.empty()) {
        for (int i = 0; i < temp.size(); ++i) {
            auto *p_float = reinterpret_cast<float *>(&temp_matrix);
            p_float[i]    = static_cast<float>(temp.at(i));
        }
        const auto &transform = Logic_entt().emplace_or_replace<Transform>(entity, temp_matrix);
        set_render_parameter(entity, "model_4x4", temp_matrix);
        return;
    }
    // matrix 与之前的内容互斥 搞定互斥的部分

    Point_3 offset            = get_offset_from_model(model, current_node_index);
    Point_3 zoom              = get_zoom_from_model(model, current_node_index);
    Eigen::Quaternionf rotate = get_rotate_from_model(model, current_node_index);
    const auto &transform     = Logic_entt().emplace_or_replace<Transform>(entity, offset, rotate, zoom);
    const auto modelMatrix    = get_model_matrix(transform);
    set_render_parameter(entity, "model_4x4", temp_matrix);
}

/**
 *
 * @param model
 * @param current_node_index
 * @param parent_node_index   好像确实没有什么用
 * @param parent_node_entity
 * @param material_base_index
 * @return
 */
entt::entity load_node_data(tinygltf::Model &model,
                            std::vector<bool> &nodes_have_deal,
                            const int current_node_index,
                            const int parent_node_index           = -1,
                            const entt::entity parent_node_entity = entt::null,
                            const int material_base_index         = 0) {
    auto node                              = model.nodes[current_node_index];
    nodes_have_deal.at(current_node_index) = true;
    if (node.mesh >= 0) {
        const entt::entity entity = Logic_entt().create();
        logic_create_proxy(entity);
        add_shader(entity,
                   "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
                   "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
                   "", "");
        auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
        // set_render_parameter(entity, "object_material", material);
        Logic_entt().emplace<Name_component>(entity, node.name);
        set_model_matrix(entity, model, current_node_index);

        // mesh 中可以有多个 Primitive, 但是其中每个 Primitive 都是必须要绘制的，而不是可选的
        get_mesh_from_gltf_model(entity, model, node.mesh, material_base_index);

        Logic_entt().emplace<Input_Component>(entity, model_3d_Event);
        world_root_add_child(entity);
        logic_update_proxy<Name_component>(entity);

        if (parent_node_entity == entt::null) {
            world_root_add_child(entity);
        } else {
            add_relation(parent_node_entity, entity);
        }
        for (const int i: node.children) {
            load_node_data(model, nodes_have_deal, i, current_node_index, entity, material_base_index);
        }
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

std::pair<PBR_component, PBR_component_ptr> load_material(tinygltf::Model &model, const int material_index) {
    // 这里函数不太对，需要修改
    const auto &material = model.materials.at(material_index);
    PBR_component pbr_material;
    PBR_component_ptr ptr;
    pbr_material.metallicFactor_  = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
    pbr_material.roughnessFactor_ = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
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

    auto ORM_function = [&](uint32_t &write_index, Texture_parameter &write_Texture, const auto texture_index) {
        const auto image_index = model.textures[texture_index].source;
        auto &image            = model.images[image_index];
        auto texture           = load_image(image);
        uint32_t index         = texture.image.get_index();
        write_index            = index;
        auto &engine           = Engine::instance();
        engine.add_bindless_texture(texture);
        write_Texture = texture;
    };

    if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const auto texture_index = material.pbrMetallicRoughness.baseColorTexture.index;
        ORM_function(pbr_material.baseColorTexture, ptr.baseColorTexture, texture_index);
    }

    if (material.normalTexture.index >= 0) {
        const auto texture_index = material.normalTexture.index;
        ORM_function(pbr_material.normalTexture, ptr.normalTexture, texture_index);
    }

    if (material.emissiveTexture.index >= 0) {
        const auto texture_index = material.emissiveTexture.index;
        ORM_function(pbr_material.emissiveTexture, ptr.emissiveTexture, texture_index);
    }

    if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0 && material.occlusionTexture.index >= 0) {
        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index == material.occlusionTexture.index) {
            const auto texture_index = material.occlusionTexture.index;
            ORM_function(pbr_material.ORM_Texture, ptr.ORM_Texture, texture_index);
        }
    } else if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        const auto texture_index = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        ORM_function(pbr_material.ORM_Texture, ptr.ORM_Texture, texture_index);
    } else if (material.occlusionTexture.index >= 0) {
        const auto texture_index = material.occlusionTexture.index;
        ORM_function(pbr_material.ORM_Texture, ptr.ORM_Texture, texture_index);
    }
    return {pbr_material, ptr};
}


entt::entity load_gltf_model(const std::string &name, const std::string &path,
                             const Point_3 offset,
                             const Eigen::Quaternionf &rotate,
                             const Point_3 zoom) {
    entt::entity entity = entt::null;
    auto optional_model = get_gltf_model(path);
    if (optional_model.has_value()) {
        auto &model          = optional_model.value();
        const auto nodes_num = model.nodes.size();
        std::vector<bool> nodes_have_deal;
        nodes_have_deal.resize(nodes_num, false);
        auto material_size            = model.materials.size();
        auto pbr_vector               = Engine::instance().get_pbr_vector();
        const int material_base_index = pbr_vector.size();
        pbr_vector.resize(material_base_index + material_size);
        // load_material(entity, model);  // 这里 然后就是 带锁的  部分的内容

        for (auto i = 0; i < nodes_num && nodes_have_deal.at(i) == false; ++i) {
            // 这里也稍微有点问题 一个节点在 children 数组中只能被引用一次（即每个节点只能有一个父亲）
            load_node_data(model, nodes_have_deal, i, -1, entt::null, material_base_index);
        }
    }
    return entity;
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

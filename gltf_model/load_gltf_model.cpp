//
// Created by 潘鑫 on 2026/3/16.
//

#include "load_gltf_model.h"
#include "input_component.h"
#include "transform_component.h"
#include "name_component.h"
#include "PBR_component.h"
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include "3d_model_display.h"

std::optional<fastgltf::Asset> get_gltf_model(const std::filesystem::path &path) {
    fastgltf::Asset model;
    static constexpr auto supportedExtensions =
            fastgltf::Extensions::KHR_mesh_quantization |
            fastgltf::Extensions::KHR_texture_transform |
            fastgltf::Extensions::MSFT_texture_dds |
            fastgltf::Extensions::KHR_materials_variants;
    fastgltf::Parser parser(supportedExtensions);
    constexpr auto gltfOptions =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
            fastgltf::Options::LoadExternalBuffers |
            // fastgltf::Options::LoadExternalImages |  // 需要注释掉这里，因为有的 image 是可选的，所以不存在
            fastgltf::Options::GenerateMeshIndices;

    auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
    if (!bool(gltfFile)) {
        std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
        return {};
    }
    auto asset = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
    if (asset.error() != fastgltf::Error::None) {
        std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error()) << '\n';
        return {};
    }

    return std::move(asset.get());
}

const unsigned char *get_accessor_start_address(fastgltf::Asset &model, fastgltf::Accessor &current_accessor) {
    const fastgltf::BufferView &bufferView = model.bufferViews[current_accessor.bufferViewIndex.value()];
    const fastgltf::Buffer &buffer         = model.buffers[bufferView.bufferIndex];
    // 数据真实起始地址 = Buffer基址 + BufferView偏移 + Accessor偏移
    unsigned char *dataPtr = nullptr;
    if (std::holds_alternative<fastgltf::sources::Vector>(buffer.data)) {
        // 100% 确定里面是 Vector
        auto &vec = std::get<fastgltf::sources::Vector>(buffer.data);
        dataPtr   = (unsigned char *) (vec.bytes.data());
    } else if (std::holds_alternative<fastgltf::sources::Array>(buffer.data)) {
        auto &arr = std::get<fastgltf::sources::Array>(buffer.data);
        dataPtr   = (unsigned char *) arr.bytes.data();
    } else if (std::holds_alternative<fastgltf::sources::URI>(buffer.data)) {
        // 100% 确定是外部路径
        auto &uri = std::get<fastgltf::sources::URI>(buffer.data);
        // std::string path = uri.uri.path().c_str(); // ⚠️记得 c_str() 抹平 pmr::string 冲突！
    }
    return dataPtr + bufferView.byteOffset + current_accessor.byteOffset;
}

std::size_t get_stride(fastgltf::Asset &model, const fastgltf::Accessor &accessor) {
    // 1. 检查 glTF JSON 文本中是否明确指定了不为 0 的有效步长（交错排列 AoS 结构）

    const auto &bufferView = model.bufferViews[accessor.bufferViewIndex.value()];

    if (bufferView.byteStride.has_value() && bufferView.byteStride.value() > 0) {
        return bufferView.byteStride.value();
    }
    // 2. 如果没有值，或者是魔法数字 0（紧凑排列 SoA 结构）
    // 采用官方工具函数自动计算当前属性自身的物理大小，完成最安全的兜底！
    // 比如：Vec3 + Float 自动返回 4 * 3 = 12 字节
    return fastgltf::getElementByteSize(accessor.type, accessor.componentType);
}

std::size_t get_stride(fastgltf::Asset &model, const int accessor_index) {
    const auto &current_accessor = model.accessors[accessor_index]; // 复制的函数需要处理
    return get_stride(model, current_accessor);
}

auto copy_indices_data(fastgltf::Asset &model, const fastgltf::Primitive &primitive) {
    auto current_accessor  = model.accessors[primitive.indicesAccessor.value()]; // 复制的函数需要处理
    const auto &bufferView = model.bufferViews[current_accessor.bufferViewIndex.value()];
    // 数据真实起始地址 = Buffer基址 + BufferView偏移 + Accessor偏移
    const unsigned char *src = get_accessor_start_address(model, current_accessor);

    std::size_t stride           = get_stride(model, current_accessor);
    std::size_t data_single_size = fastgltf::getElementByteSize(current_accessor.type,
                                                                current_accessor.componentType);
    share_block result;
    // auto address       = malloc(current_accessor.count * data_single_size);
    result.ptr         = std::make_shared<char[]>(current_accessor.count * data_single_size);
    result.count       = current_accessor.count;
    result.single_size = data_single_size;
    result.total_size  = current_accessor.count * data_single_size;
    result.data        = result.ptr.get();
    if (data_single_size == stride) {
        memcpy(result.data, src, result.total_size); // todo
    } else {
        // 有间隔，需要做一些其他处理
    }
    return result;
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
        memcpy(dst_address, attribute.data_ptr + index * attribute.element_stride, attribute.element_size);
    dst_address += attribute.element_size;
    return dst_address;
}

void read_attribute(fastgltf::Asset &model, fastgltf::Accessor &accessor, Attribute &attribute) {
    attribute.data_ptr       = get_accessor_start_address(model, accessor);
    attribute.element_size   = fastgltf::getElementByteSize(accessor.type, accessor.componentType);
    attribute.element_stride = get_stride(model, accessor);
    attribute.element_count  = accessor.count;
}

auto mem_copy_all_attributes(const uint32_t count, const std::vector<Attribute> &attributes) {
    share_block result;
    auto single_size = 0;
    for (const auto &attribute: attributes) {
        single_size += attribute.element_size;
    }
    result.ptr         = std::make_shared<char[]>(count * single_size);
    result.count       = count;
    result.single_size = single_size;
    result.total_size  = result.count * result.single_size;
    result.data        = result.ptr.get();
    auto dst_address   = static_cast<unsigned char *>(result.data);
    for (size_t i = 0; i < count; ++i) {
        for (const auto &attribute: attributes) {
            dst_address = memcpy_attribute(dst_address, attribute, i);
        }
    }
    return result;
}


auto copy_vertices_data(size_t size, fastgltf::Asset &model, const fastgltf::Primitive &primitive) {
    Attribute position   = {"position", nullptr, 12, 0};
    Attribute normal     = {"normal", nullptr, 12, 0};
    Attribute texcoord_0 = {"texcoord_0", nullptr, 8, 0};


    // 2. 获取顶点属性（如位置、法线、纹理坐标）
    {
        for (const auto &attribute: primitive.attributes) {
            if (attribute.name == "POSITION") {
                read_attribute(model, model.accessors[attribute.accessorIndex], position);
            }
        }
    } {
        for (const auto &attribute: primitive.attributes) {
            if (attribute.name == "NORMAL") {
                read_attribute(model, model.accessors[attribute.accessorIndex], normal);
            }
        }
    } {
        for (const auto &attribute: primitive.attributes) {
            if (attribute.name == "TEXCOORD_0") {
                read_attribute(model, model.accessors[attribute.accessorIndex], texcoord_0);
            }
        }
    }

    std::vector<Attribute> attributes;
    attributes.push_back(position);
    attributes.push_back(normal);
    attributes.push_back(texcoord_0);

    std::vector<std::string> find_strings;
    // find_strings.push_back("JOINTS_0");
    // find_strings.push_back("WEIGHTS_0");
    // for (auto find_string: find_strings) {
    //     auto it = primitive.attributes.find(find_string);
    //     if (it != primitive.attributes.end()) {
    //         Attribute attribute_temp;
    //         attribute_temp.name = find_string;
    //         read_attribute(model, model.accessors[it->second], attribute_temp);
    //         attributes.push_back(attribute_temp);
    //     }
    // }
    return mem_copy_all_attributes(position.element_count, attributes);
    // 上面的做法应该是 有几个类型就复制几个属性，没有就跳过
}


void get_mesh_from_gltf_model(entt::entity entity, fastgltf::Asset &model, const std::size_t mesh_index) {
    // std::vector<VKR_Primitive> // 如果可以的话，尽可能在这里搞定，之后只需要复制一下就好
    const auto mesh             = model.meshes[mesh_index];
    size_t indices_memory_size  = 0;
    size_t vertices_memory_size = 0;
    for (const auto &primitive: mesh.primitives) {
        // 最开始需要能够确定数量
        if (primitive.indicesAccessor.has_value()) {
            const auto current_accessor  = model.accessors[primitive.indicesAccessor.value()]; // 复制的函数需要处理
            std::size_t data_single_size = fastgltf::getElementByteSize(current_accessor.type,
                                                                        current_accessor.componentType);
            indices_memory_size += current_accessor.count * data_single_size;
        }

        for (const auto &attribute: primitive.attributes) {
            const auto current_accessor        = model.accessors[attribute.accessorIndex]; // 复制的函数需要处理
            const std::size_t data_single_size = fastgltf::getElementByteSize(current_accessor.type,
                                                                              current_accessor.componentType);
            vertices_memory_size += current_accessor.count * data_single_size;
        }
    }

    for (const auto &primitive: mesh.primitives) {
        if (primitive.indicesAccessor.has_value()) {
            auto result = copy_indices_data(model, primitive);
            Logic_entt().get_or_emplace<Geometry_data>(entity).push_indices(result);
        }
        auto result = copy_vertices_data(vertices_memory_size, model, primitive);
        Logic_entt().get_or_emplace<Geometry_data>(entity).push_vertices(result);
    }
    // std::vector<uint32_t> material_index;
    // for (const auto &primitive: mesh.primitives) {
    //     if (primitive.material > -1) {
    //         const auto result = load_material(model, primitive.material);
    //         auto &manager     = Engine::instance().get_pbr_manager();
    //         auto index        = manager.push(result.first, result.second);
    //         material_index.push_back(index);
    //         // index 给出了那么应该写到哪里呢？
    //     }
    //     break;
    // }

    // auto bound_box = find_min_max_point(sp_vertices);
    // auto &AABB     = Logic_entt().get_or_emplace<AABB_min_max<Point_3> >(entity, bound_box);
    // 这里呢？ 也是应该怎么做的问题

    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    // if (primitives.size() == material_index.size()) {
    //     int i = 0;
    //     for (auto &primitive: primitives) {
    //         if (primitive.index_type == VK_INDEX_TYPE_MAX_ENUM) {
    //             primitive.vertex_command.firstInstance = material_index.at(i);
    //         } else
    //             primitive.indexed_command.firstInstance = material_index.at(i);
    //         ++i;
    //         break;
    //     }
    // }

    logic_update_proxy(entity, primitives); //  这里还是能改一些内容的
    logic_update_add_tag<opacity_tag>(entity);
}


void add_Transform_parameter(const entt::entity entity, const fastgltf::Node &node) {
    if (std::holds_alternative<fastgltf::TRS>(node.transform)) {
        auto &trs = std::get<fastgltf::TRS>(node.transform);
        Logic_entt().emplace<Transform>(entity,
                                        Point_3{
                                            trs.translation.x(), trs.translation.y(), trs.translation.z()
                                        },
                                        Eigen::Quaternionf{
                                            trs.rotation.w(), trs.rotation.x(), trs.rotation.y(),
                                            trs.rotation.z()
                                        },
                                        Point_3{trs.scale.x(), trs.scale.y(), trs.scale.z()});
    } else if (std::holds_alternative<fastgltf::math::fmat4x4>(node.transform)) {
        auto &trs = std::get<fastgltf::math::fmat4x4>(node.transform);
        // 列存储
        const auto modelMatrix          = Eigen::Map<const Eigen::Matrix<float, 4, 4, Eigen::ColMajor>>(trs.data());
        Eigen::Matrix3f rotation_matrix = modelMatrix.block<3, 3>(0, 0);
        Eigen::Quaternionf rotation     = Eigen::Quaternionf(rotation_matrix);;
        Logic_entt().emplace<Transform>(entity,
                                        Point_3{
                                            modelMatrix(0, 3),
                                            modelMatrix(1, 3),
                                            modelMatrix(2, 3)
                                        },
                                        rotation,
                                        Point_3{
                                            modelMatrix.col(0).head<3>().norm(),
                                            modelMatrix.col(1).head<3>().norm(),
                                            modelMatrix.col(2).head<3>().norm(),
                                        });
    }
    Logic_entt().emplace_or_replace<Transform_matrix_dirty>(entity);
}

/**
 *
 * @param model
 * @param current_node_index
 * @param parent_node_index   好像确实没有什么用
 * @param parent_entity
 * @return
 */
entt::entity load_node_data(fastgltf::Asset &model,
                            std::vector<bool> &nodes_have_deal,
                            const int current_node_index,
                            const int parent_node_index      = -1,
                            const entt::entity parent_entity = entt::null) {
    auto node                              = model.nodes[current_node_index];
    nodes_have_deal.at(current_node_index) = true;
    const entt::entity entity              = Logic_entt().create();
    Logic_entt().emplace<Name_component>(entity, node.name.c_str());
    if (parent_entity == entt::null) world_root_add_child(entity);
    else add_relation(parent_entity, entity);


    add_Transform_parameter(entity, node);

    if (node.meshIndex.has_value()) {
        logic_create_proxy(entity); // 有几何的时候才创造吗？
        Logic_entt().emplace<shader_data>(entity, Engine::instance().get_gltf_shader_data());
        logic_update_proxy<shader_data>(entity);
        auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
        // set_render_parameter(entity, "object_material", material);
        get_mesh_from_gltf_model(entity, model, node.meshIndex.value());
        Logic_entt().emplace<Input_Component>(entity, model_3d_Event);
        logic_update_proxy<Name_component>(entity);
    }
    if (node.cameraIndex.has_value()) {
        LOG_INFO(g_log(), "need deal node  camera ");
    }
    if (node.lightIndex.has_value()) {
        LOG_INFO(g_log(), "need deal node  light ");
    }
    if (node.skinIndex.has_value()) {
        LOG_INFO(g_log(), "need deal node  skin ");
    }
    // if (node.skinIndex.has_value()) {
    //     LOG_INFO(g_log(), "need deal node  emitter ");
    // }

    for (const auto i: node.children) {
        load_node_data(model, nodes_have_deal, i, current_node_index, entity);
    }

    return entt::null;
}


Texture_parameter load_image(fastgltf::Image &image) {
    return {};
}


entt::entity load_gltf_model(const std::string &name, const std::filesystem::path &path,
                             const Point_3 offset,
                             const Eigen::Quaternionf &rotate,
                             const Point_3 zoom) {
    auto optional_model = get_gltf_model(path);
    if (optional_model.has_value()) {
        const entt::entity model_entity = Logic_entt().create();
        Logic_entt().emplace<Name_component>(model_entity, name);
        world_root_add_child(model_entity);
        auto &model          = optional_model.value();
        const auto nodes_num = model.nodes.size();
        std::vector<bool> nodes_have_deal;
        nodes_have_deal.resize(nodes_num, false);
        size_t has_mesh = 0;
        for (auto i = 0; i < nodes_num && nodes_have_deal.at(i) == false; ++i) {
            auto node = model.nodes[i];
            if (node.meshIndex.has_value()) {
                has_mesh++;
            }
        }
        for (auto scene: model.scenes) {
            const entt::entity entity = Logic_entt().create();
            Logic_entt().emplace<Name_component>(entity, scene.name.c_str());
            add_relation(model_entity, entity);
            for (const auto node_index: scene.nodeIndices) {
                load_node_data(model, nodes_have_deal, node_index, -1, entity);
            }
        }
        return model_entity;

        // for (auto i = 0; i < nodes_num && i < 1000 && nodes_have_deal.at(i) == false; ++i) {
        //     // 这里也稍微有点问题 一个节点在 children 数组中只能被引用一次（即每个节点只能有一个父亲）
        //     load_node_data(model, nodes_have_deal, i, -1, entt::null);
        // }
    }
    return entt::null;
}

VkPrimitiveTopology get_primitive_topology(const fastgltf::Primitive &primitive) {
    switch (primitive.type) {
        case fastgltf::PrimitiveType::Points:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case fastgltf::PrimitiveType::Lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case fastgltf::PrimitiveType::LineLoop:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case fastgltf::PrimitiveType::Triangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case fastgltf::PrimitiveType::TriangleStrip:
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

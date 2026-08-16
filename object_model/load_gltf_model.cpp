//
// Created by 潘鑫 on 2026/3/16.
//

#include "load_gltf_model.h"
#include "input_component.h"
#include "transform_component.h"
#include "name_component.h"
#include "../render/render_common/PBR_component.h"
#include "3d_model_display.h"
#include "camera_optical_component.h"
#include "GPU_frustum_cull.h"
#include "scene_component.h"
#include "stb_image.h"
#include "time_measure.h"
#include "tinyddsloader.h"
#include "transform_AABB.h"
#include "world_scene_root.h"

std::optional<fastgltf::Asset> get_gltf_model(const std::filesystem::path &path) {
    fastgltf::Asset model;
    static constexpr auto supportedExtensions =
            fastgltf::Extensions::KHR_mesh_quantization |
            fastgltf::Extensions::EXT_meshopt_compression |
            fastgltf::Extensions::KHR_texture_transform |
            fastgltf::Extensions::MSFT_texture_dds |
            fastgltf::Extensions::KHR_texture_basisu |
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
    if (data_single_size == 2) {
        result.ptr         = std::make_shared<char[]>(current_accessor.count * data_single_size * 2);
        result.count       = current_accessor.count;
        result.single_size = data_single_size * 2;
        result.total_size  = current_accessor.count * data_single_size * 2;
        result.data        = result.ptr.get();
        if (data_single_size == stride) {
            memcpy(result.data, src, result.total_size);
        }
        if (data_single_size == stride) {
            uint32_t *dst          = (uint32_t *) result.data;
            uint16_t *uint16_t_src = (uint16_t *) src;
            for (uint32_t i = 0; i < current_accessor.count; i++) {
                *dst = *uint16_t_src;
                dst++;
                uint16_t_src++;
                // 完全更改为了 uint32_t
            }
        } else {
            // 有间隔，需要做一些其他处理
        }
    } else if (data_single_size == 4) {
        result.ptr         = std::make_shared<char[]>(current_accessor.count * data_single_size);
        result.count       = current_accessor.count;
        result.single_size = data_single_size;
        result.total_size  = current_accessor.count * data_single_size;
        result.data        = result.ptr.get();
        if (data_single_size == stride) {
            memcpy(result.data, src, result.total_size);
        }
        if (data_single_size == stride) {
            memcpy(result.data, src, result.total_size);
        } else {
            // 有间隔，需要做一些其他处理
        }
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
    find_strings.push_back("JOINTS_0"); // 有时候是四字节, 有时候是 8 字节
    find_strings.push_back("WEIGHTS_0");
    for (auto find_string: find_strings) {
        for (const auto &attribute: primitive.attributes) {
            if (attribute.name == std::string_view(find_string)) {
                Attribute attribute_temp;
                attribute_temp.name = find_string;
                read_attribute(model, model.accessors[attribute.accessorIndex], attribute_temp);
                attributes.push_back(attribute_temp);
                break;
            }
        }
    }
    // 然后这里就有问题了 , 我之前设置的都是 三个 数据,长度是固定的,
    // 现在需要更改了
    return mem_copy_all_attributes(position.element_count, attributes);
    // 上面的做法应该是 有几个类型就复制几个属性，没有就跳过
}

/**
 *  创建连续的索引  0 1 2 ..... number
 * @param number_of_indices
 * @return
 */
auto create_continue_indices(size_t number_of_indices) {
    if (number_of_indices % 3 != 0) {
        number_of_indices = number_of_indices - number_of_indices % 3;
    }
    share_block result;
    result.count       = number_of_indices;
    result.single_size = 4;
    result.total_size  = result.count * result.single_size;
    result.ptr         = std::make_shared<char[]>(result.total_size);
    result.data        = result.ptr.get();
    uint32_t *dst      = (uint32_t *) result.data;
    for (uint32_t i = 0; i < result.count; i++) {
        *dst = i;
        dst++;
    }
    return result;
}

void get_mesh_from_gltf_model(entt::entity entity, fastgltf::Asset &model, const std::size_t mesh_index) {
    // std::vector<VKR_Primitive> // 如果可以的话，尽可能在这里搞定，之后只需要复制一下就好
    const auto mesh             = model.meshes[mesh_index];
    size_t indices_memory_size  = 0;
    size_t vertices_memory_size = 0;
    size_t number_of_vertices   = 0;
    for (const auto &primitive: mesh.primitives) {
        // 最开始需要能够确定数量
        if (primitive.indicesAccessor.has_value()) {
            const auto current_accessor        = model.accessors[primitive.indicesAccessor.value()]; // 复制的函数需要处理
            const std::size_t data_single_size = fastgltf::getElementByteSize(current_accessor.type,
                                                                              current_accessor.componentType);
            indices_memory_size += current_accessor.count * data_single_size;
        }

        for (const auto &attribute: primitive.attributes) {
            const auto current_accessor        = model.accessors[attribute.accessorIndex]; // 复制的函数需要处理
            const std::size_t data_single_size = fastgltf::getElementByteSize(current_accessor.type,
                                                                              current_accessor.componentType);
            vertices_memory_size += current_accessor.count * data_single_size;
            number_of_vertices   = current_accessor.count;
        }
        // 这里的时候才开始确定 material
        if (primitive.materialIndex.has_value())
            Logic_entt().get_or_emplace<Geometry_data>(entity).push_material(primitive.materialIndex.value());
    }
    for (const auto &primitive: mesh.primitives) {
        if (primitive.indicesAccessor.has_value()) {
            auto result = copy_indices_data(model, primitive);
            Logic_entt().get_or_emplace<Geometry_data>(entity).push_indices(result);
        } else {
            auto result = create_continue_indices(number_of_vertices);
        }
        auto result          = copy_vertices_data(vertices_memory_size, model, primitive);
        const auto bound_box = find_min_max_point(result);

        Logic_entt().get_or_emplace<Geometry_data>(entity).push_vertices(result, bound_box);
    }

    Logic_entt().emplace_or_replace<Geometry_data_need_copy_tag>(entity);
    Logic_entt().emplace_or_replace<opacity_tag>(entity);
}


void add_Transform_parameter(const entt::entity entity, const fastgltf::Node &node) {
    if (std::holds_alternative<fastgltf::TRS>(node.transform)) {
        auto &trs = std::get<fastgltf::TRS>(node.transform);
        Logic_entt().emplace<Transform>(entity,
                                        Eigen::Vector3f{
                                            trs.translation.x(), trs.translation.y(), trs.translation.z()
                                        },
                                        Eigen::Quaternionf{
                                            trs.rotation.w(), trs.rotation.x(), trs.rotation.y(),
                                            trs.rotation.z()
                                        },
                                        Eigen::Vector3f{trs.scale.x(), trs.scale.y(), trs.scale.z()});
    } else if (std::holds_alternative<fastgltf::math::fmat4x4>(node.transform)) {
        auto &trs = std::get<fastgltf::math::fmat4x4>(node.transform);
        // 列存储
        const auto modelMatrix          = Eigen::Map<const Eigen::Matrix<float, 4, 4, Eigen::ColMajor>>(trs.data());
        Eigen::Matrix3f rotation_matrix = modelMatrix.block<3, 3>(0, 0);
        Eigen::Quaternionf rotation     = Eigen::Quaternionf(rotation_matrix);;
        Logic_entt().emplace<Transform>(entity,
                                        Eigen::Vector3f{
                                            modelMatrix(0, 3),
                                            modelMatrix(1, 3),
                                            modelMatrix(2, 3)
                                        },
                                        rotation,
                                        Eigen::Vector3f{
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
                            std::vector<entt::entity> &nodes_have_deal,
                            const size_t current_node_index,
                            const size_t parent_node_index   = -1,
                            const entt::entity parent_entity = entt::null) {
    auto node                              = model.nodes[current_node_index];
    const entt::entity entity              = Logic_entt().create();
    nodes_have_deal.at(current_node_index) = entity;
    Logic_entt().emplace<Name_component>(entity, node.name.c_str());
    if (parent_entity == entt::null) world_root_add_child(entity);
    else add_relation(parent_entity, entity);
    add_Transform_parameter(entity, node);
    if (node.meshIndex.has_value()) {
        get_mesh_from_gltf_model(entity, model, node.meshIndex.value());
    }


    if (node.cameraIndex.has_value()) {
        auto &camera = model.cameras[node.cameraIndex.value()];
        if (std::holds_alternative<fastgltf::Camera::Orthographic>(camera.camera)) {
        } else if (std::holds_alternative<fastgltf::Camera::Perspective>(camera.camera)) {
            auto &data        = std::get<fastgltf::Camera::Perspective>(camera.camera);
            float aspectRatio = 1.0f;
            float zfar        = 1000.0f;
            if (data.aspectRatio.has_value()) {
                aspectRatio = data.aspectRatio.value();
            } else if (data.zfar.has_value()) {
                zfar = data.zfar.value();
            }
            Logic_entt().emplace<camera_optical_component>(entity,
                                                           data.yfov,
                                                           data.znear,
                                                           aspectRatio,
                                                           zfar);
        }

        LOG_INFO(g_log(), "need deal node  camera ");
    }
    if (node.lightIndex.has_value()) {
        auto light_data   = model.lights[node.lightIndex.value()];
        auto &logic_light = Logic_entt().emplace<Light>(entity);
        logic_light.set_color(light_data.color.x(), light_data.color.y(), light_data.color.z());
        logic_light.ser_color_type(float(light_data.type));
        logic_light.set_intensity(light_data.intensity);
        if (light_data.range.has_value())
            logic_light.set_range(light_data.range.value());
        if (light_data.innerConeAngle.has_value())
            logic_light.set_innerConeAngle(light_data.innerConeAngle.value());
        if (light_data.outerConeAngle.has_value())
            logic_light.set_outerConeAngle(light_data.outerConeAngle.value());


        LOG_INFO(g_log(), "need deal node  light ");
    }
    if (node.skinIndex.has_value()) {
        LOG_INFO(g_log(), "need deal node  skin ");
        auto skin_data = model.skins[node.skinIndex.value()];
    }
    // Animation
    // Sampler
    //
    // Material
    // Texture
    // Sampler
    // Image

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


// Logic_entt().emplace<std::vector<Animation_entity> >(root_entity, animations);

/**
 *
 * sampler.inputAccessor：指向时间轴（Timeline）。
 *    这是一个一维的浮点数数组（如 [0.0s, 0.5s, 1.0s]），代表每个关键帧发生的时间点。
 * sampler.outputAccessor：指向变换数值（Transform Values）。
 *    根据通道的不同，它可能是 vec3（位置/缩放）或 vec4（四元数旋转）。
 * sampler.interpolation：一个枚举值，告诉引擎在两个关键帧之间应该如何平滑过渡
 *    fastgltf::Interpolation::Linear（线性插值）
 *    fastgltf::Interpolation::Step（阶跃，用于定格动画/开关状态）
 *    fastgltf::Interpolation::CubicSpline（三次方样条插值，曲线更平滑）
 * @param model
 * @param nodes_have_deal
 * @param root_entity
 */
void gltf_load_animal(const fastgltf::Asset &model,
                      const std::vector<entt::entity> &nodes_have_deal,
                      const entt::entity &root_entity) {
    if (model.animations.empty()) {
        return;
    }
    std::vector<RuntimeAnimation> animations;
    animations.reserve(model.animations.size());


    for (const auto &animation: model.animations) {
        // animation.name.c_str(); 这里有动作的名字
        // 有多个不同类型的 动作
        float max_frame_time = 0.0f;
        std::vector<RuntimeChannel> channels;
        channels.reserve(animation.channels.size());
        for (const auto &channel: animation.channels) {
            RuntimeChannel temp_channel;
            if (channel.nodeIndex.has_value()) {
                // 拿到了 nodeIndex
                // 受影响的 glTF 节点（Node）全局索引
                // 那个受会受影响,需要
                temp_channel.effect_entity = nodes_have_deal[channel.nodeIndex.value()];
            } else {
                temp_channel.effect_entity = entt::null;
            }
            temp_channel.path                      = channel.path;
            auto sampler                           = animation.samplers[channel.samplerIndex];
            const fastgltf::Accessor &timeAccessor = model.accessors[sampler.inputAccessor];
            temp_channel.keyframeTimes.reserve(timeAccessor.count);
            fastgltf::iterateAccessor<float>(model, timeAccessor, [&](float timeValue) {
                temp_channel.keyframeTimes.push_back(timeValue);
            });
            max_frame_time             = std::max(max_frame_time, temp_channel.keyframeTimes.back());
            temp_channel.interpolation = sampler.interpolation;

            const fastgltf::Accessor &outputAccessor = model.accessors[sampler.outputAccessor];
            if (outputAccessor.type == fastgltf::AccessorType::Vec4) {
                std::vector<Eigen::Quaternionf> rotates;
                rotates.reserve(outputAccessor.count);
                auto function = [&](fastgltf::math::f32vec4 v4) {
                    rotates.emplace_back(v4.w(), v4.x(), v4.y(), v4.z());
                };
                fastgltf::iterateAccessor<fastgltf::math::f32vec4>(model, outputAccessor, function);
                temp_channel.offset_rotate = rotates;
            } else if (outputAccessor.type == fastgltf::AccessorType::Vec3) {
                std::vector<Eigen::Vector3f> offset;
                offset.reserve(outputAccessor.count);
                auto function = [&](fastgltf::math::f32vec3 v3) {
                    offset.emplace_back(v3.x(), v3.y(), v3.z());
                };
                fastgltf::iterateAccessor<fastgltf::math::f32vec3>(model, outputAccessor, function);
                temp_channel.offset_rotate = offset;
            }
            channels.push_back(temp_channel);
            // channel.samplerIndex  这里是什么的意思 ?
            // channel.samplerIndex  并不是一一 对应的 , 最简单的办法, 那就 完全复制 一条
            // sampler.inputAccessor 大概率是同一条 ,但是也是存在不是同一条的情况
            // 好像消息是 目前只 剩 CPU 部分需要去做了,坏消息是,很难做
        }
        RuntimeAnimation temp{animation.name.c_str(), max_frame_time, channels};
        animations.push_back(temp);
    }
    Logic_entt().emplace<std::vector<RuntimeAnimation> >(root_entity, animations);
}


using Skin_matrix_vector_index = std::vector<entt::entity>;

void gltf_load_skin(const fastgltf::Asset &model,
                    const std::vector<entt::entity> &nodes_have_deal,
                    const entt::entity &root_entity) {
    for (auto skin: model.skins) {
        if (skin.skeleton.has_value()) {
            size_t rootNodeIdx = skin.skeleton.value();
            std::cout << "  Skeleton Root Node Index: " << rootNodeIdx << "\n";
            // 指向整个骨骼关节层级树（Joints Hierarchy）的公共根节点

            const auto &transform       = Logic_entt().get<Transform>(nodes_have_deal[1]);
            Eigen::Matrix4f mesh_matrix = transform.get_transform_matrix();
            mesh_matrix                 = mesh_matrix.inverse().eval();
            Logic_entt().emplace<Inverse_Global_Transform>(root_entity, mesh_matrix);
        }
        if (skin.inverseBindMatrices.has_value()) {
            size_t accessorIdx                 = skin.inverseBindMatrices.value();
            const fastgltf::Accessor &accessor = model.accessors[accessorIdx];
            if (accessor.type == fastgltf::AccessorType::Mat4) {
                auto &matrixData = Logic_entt().emplace<std::vector<
                    Eigen::Matrix4f> >(root_entity, accessor.count);

                size_t count  = 0;
                auto function = [&](fastgltf::math::fmat4x4 mat) {
                    const auto modelMatrix = Eigen::Map<const Eigen::Matrix<float, 4, 4, Eigen::ColMajor>>
                            (mat.data());
                    matrixData[count++] = modelMatrix;
                };
                fastgltf::iterateAccessor<fastgltf::math::fmat4x4>(model, accessor, function);

                Skin_matrix_vector_index skin_joints;
                for (auto i = 0; i < skin.joints.size(); ++i) {
                    auto joint  = skin.joints[i];
                    auto entity = nodes_have_deal[joint];
                    Logic_entt().emplace<InverseBindMatrix>(entity, matrixData[i]);
                    skin_joints.push_back(entity);
                }
                Logic_entt().emplace<Skin_matrix_vector_index>(root_entity, skin_joints);
            }
        } else {
            // 如果 glTF 没提供 IBM，根据规范，所有关节默认使用单位矩阵 (Identity Matrix)
            std::cout << "  No Inverse Bind Matrices found. Using identity matrices.\n";
        }
    }
}


void gltf_update_joint_matrix(const entt::entity &model_entity) {
    auto update_joint_matrix = [](const entt::entity entity) {
        if (Logic_entt().all_of<Transform_Matrix, Scene_Component, InverseBindMatrix>(entity)) {
            auto transform_matrix           = Logic_entt().get<Transform_Matrix>(entity);
            const auto &inverse_bind_matrix = Logic_entt().get<InverseBindMatrix>(entity);
            Eigen::Matrix4f result          = transform_matrix * inverse_bind_matrix.matrix;
            Logic_entt().emplace_or_replace<JointMatrix>(entity, result);
        }
    };
    add_recursion_function_to_children(model_entity, update_joint_matrix);

    if (auto skin_joints = Logic_entt().try_get<Skin_matrix_vector_index>(model_entity)) {
        std::vector<Eigen::Matrix4f> JointMatrices;
        for (const auto entity: *skin_joints) {
            JointMatrices.push_back(Logic_entt().get<JointMatrix>(entity).matrix);
        }
        const auto matrix_ptr = JointMatrices.data();
        auto matrix_size      = JointMatrices.size() * sizeof(Eigen::Matrix4f);
        auto matrix_buffer    = copy_data_to_SSBO_buffer(matrix_ptr, matrix_size);
        set_render_parameter(model_entity, "JointMatrices", matrix_buffer);
    }
}


void update_joint_matrix_matrix(const entt::entity entity) {
    if (Logic_entt().all_of<Transform_Matrix, Scene_Component, InverseBindMatrix>(entity)) {
        auto transform_matrix           = Logic_entt().get<Transform_Matrix>(entity);
        const auto &inverse_bind_matrix = Logic_entt().get<InverseBindMatrix>(entity);
        Eigen::Matrix4f result          = transform_matrix * inverse_bind_matrix.matrix;
        Logic_entt().emplace_or_replace<JointMatrix>(entity, result);
    }
};


Picture_parameters loadImage(const std::filesystem::path &path, const fastgltf::Asset &model, fastgltf::Image &image) {
    Picture_parameters picture_parameters{};
    std::visit(fastgltf::visitor{
                   [](auto &arg) {
                   },
                   [&](fastgltf::sources::URI &filePath) {
                       assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                       assert(filePath.uri.isLocalPath());

                       std::filesystem::path baseDir = path.parent_path();

                       // We're only capable of loading local files.

                       const std::string relativePathStr(filePath.uri.path().begin(),
                                                         filePath.uri.path().end());
                       std::filesystem::path absolutePath =
                               std::filesystem::weakly_canonical(baseDir / relativePathStr);

                       std::string ext = absolutePath.extension().string();

                       if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                           ScopedTimer temp("stbi_load");
                           unsigned char *data = stbi_load(absolutePath.c_str(),
                                                           &picture_parameters.width,
                                                           &picture_parameters.height,
                                                           &picture_parameters.channels,
                                                           4);
                           picture_parameters.channels   = 4;
                           picture_parameters.image_data = data;
                       } else if (ext == ".dds") {
                           tinyddsloader::DDSFile dds;
                           auto ret = dds.Load(absolutePath.c_str());
                           if (tinyddsloader::Result::Success == ret) {
                           }
                       }
                   },
                   [&](fastgltf::sources::Array &vector) {
                       int width, height, nrChannels;
                       ScopedTimer temp("stbi_load");
                       unsigned char *data =
                               stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(vector.bytes.
                                                         data()),
                                                     static_cast<int>(vector.bytes.size()),
                                                     &picture_parameters.width,
                                                     &picture_parameters.height,
                                                     &picture_parameters.channels,
                                                     4);
                       picture_parameters.channels   = 4;
                       picture_parameters.image_data = data;
                   },
                   [&](fastgltf::sources::BufferView &view) {
                       auto &bufferView = model.bufferViews[view.bufferViewIndex];
                       auto &buffer     = model.buffers[bufferView.bufferIndex];
                       // Yes, we've already loaded every buffer into some GL buffer. However, with GL it's simpler
                       // to just copy the buffer data again for the texture. Besides, this is just an example.
                       std::visit(fastgltf::visitor{
                                      // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                                      // all buffers are already loaded into a vector.
                                      [&](const fastgltf::sources::Array &vector) {
                                          ScopedTimer temp("stbi_load");
                                          int width, height, nrChannels;
                                          unsigned char *data =
                                                  stbi_load_from_memory(reinterpret_cast<const
                                                                            stbi_uc *>(
                                                                            vector.bytes.data() + bufferView.
                                                                            byteOffset),
                                                                        static_cast<int>(bufferView.byteLength),
                                                                        &picture_parameters.width,
                                                                        &picture_parameters.height,
                                                                        &picture_parameters.channels,
                                                                        4);
                                          picture_parameters.channels   = 4;
                                          picture_parameters.image_data = data;
                                      },
                                      [](auto &arg) {
                                          uint8_t *data;
                                      }
                                  }, buffer.data);
                   },
               }, image.data);
    return picture_parameters;
}


auto load_texture_info(const std::filesystem::path &path,
                       const fastgltf::Asset &model,
                       const fastgltf::TextureInfo &texture_info) {
    if (texture_info.textureIndex < model.textures.size()) {
        auto texture = model.textures[texture_info.textureIndex];

        if (texture.basisuImageIndex.has_value() &&
            texture.basisuImageIndex.value() <= model.images.size()) {
            auto image   = model.images[texture.basisuImageIndex.value()];
            auto picture = loadImage(path, model, image);
        } else if (texture.ddsImageIndex.has_value() &&
                   texture.ddsImageIndex.value() <= model.images.size()) {
            auto image = model.images[texture.ddsImageIndex.value()];

            tinyddsloader::DDSFile dds;
            if (std::holds_alternative<fastgltf::sources::URI>(image.data)) {
                auto &filePath = std::get<fastgltf::sources::URI>(image.data);
                assert(filePath.fileByteOffset == 0);
                assert(filePath.uri.isLocalPath());
                std::filesystem::path baseDir = path.parent_path();
                const std::string relativePathStr(filePath.uri.path().begin(),
                                                  filePath.uri.path().end());
                std::filesystem::path absolutePath =
                        std::filesystem::weakly_canonical(baseDir / relativePathStr);
                std::string ext = absolutePath.extension().string();
                if (ext == ".dds") {
                    auto ret = dds.Load(absolutePath.c_str());
                    if (tinyddsloader::Result::Success == ret) {
                        auto result = load_dds_to_gpu(dds);
                        return result;
                    }
                }
            }
        } else if (texture.imageIndex.has_value() && texture.imageIndex.value() <= model.images.size()) {
            auto image   = model.images[texture.imageIndex.value()];
            auto picture = loadImage(path, model, image);
            auto result  = create_2d_texture(picture);
            Engine::instance().add_bindless_texture(result);
            return result;
        }
    }
}


void load_materials(std::vector<uint32_t> &material_indices,
                    const std::filesystem::path &path,
                    const fastgltf::Asset &model) {
    ScopedTimer temp("load_materials");
    auto &pbr_manager = Engine::instance().get_pbr_manager();
    for (const auto &material: model.materials) {
        PBR_component pbr;
        PBR_Texture_ptr ptr;
        pbr.alphaCutoff        = material.alphaCutoff;
        pbr.doubleSided        = material.doubleSided;
        pbr.alphaMode          = static_cast<uint32_t>(material.alphaMode);
        pbr.baseColorFactor_.R = material.pbrData.baseColorFactor[0];
        pbr.baseColorFactor_.G = material.pbrData.baseColorFactor[1];
        pbr.baseColorFactor_.B = material.pbrData.baseColorFactor[2];
        pbr.emissiveFactor_.R  = material.emissiveFactor[0];
        pbr.emissiveFactor_.G  = material.emissiveFactor[1];
        pbr.emissiveFactor_.B  = material.emissiveFactor[2];
        pbr.metallicFactor_    = material.pbrData.metallicFactor;
        pbr.roughnessFactor_   = material.pbrData.roughnessFactor;
        // pbr.ior                = material.ior;
        if (material.occlusionTexture.has_value() && material.pbrData.metallicRoughnessTexture.has_value()) {
            if (material.occlusionTexture.value().textureIndex ==
                material.pbrData.metallicRoughnessTexture.value().textureIndex) {
                pbr.occlusion_strength_ = material.occlusionTexture.value().strength;
                auto texture            = load_texture_info(path, model, material.occlusionTexture.value());
                pbr.ORM_Texture         = texture.image.get_index();
                ptr.ORM_Texture         = texture;
            } else {
                auto texture    = load_texture_info(path, model, material.pbrData.metallicRoughnessTexture.value());
                pbr.ORM_Texture = texture.image.get_index();
                ptr.ORM_Texture = texture;
                // 否则的话,就需要 想办法合并两个通道的 内容 了
            }
        } else if (material.occlusionTexture.has_value()) {
            pbr.occlusion_strength_ = material.occlusionTexture.value().strength;
            auto texture            = load_texture_info(path, model, material.occlusionTexture.value());
            pbr.ORM_Texture         = texture.image.get_index();
            ptr.ORM_Texture         = texture;
        } else if (material.pbrData.metallicRoughnessTexture.has_value()) {
            auto texture    = load_texture_info(path, model, material.pbrData.metallicRoughnessTexture.value());
            pbr.ORM_Texture = texture.image.get_index();
            ptr.ORM_Texture = texture;
        }
        if (material.normalTexture.has_value()) {
            auto texture      = load_texture_info(path, model, material.normalTexture.value());
            pbr.normalTexture = texture.image.get_index();
            ptr.normalTexture = texture;
        }
        if (material.emissiveTexture.has_value()) {
            auto texture        = load_texture_info(path, model, material.emissiveTexture.value());
            pbr.emissiveTexture = texture.image.get_index();
            ptr.emissiveTexture = texture;
        }
        if (material.pbrData.baseColorTexture.has_value()) {
            auto texture         = load_texture_info(path, model, material.pbrData.baseColorTexture.value());
            pbr.baseColorTexture = texture.image.get_index();
            ptr.baseColorTexture = texture;
        }
        auto material_index = pbr_manager.push(pbr, ptr); // 总之,最后 ,需要写到这里的
        material_indices.push_back(material_index);
    }
}


void update_material(entt::entity model_entity) {
    {
        std::vector<uint32_t> temp;
        auto material_indices    = Logic_entt().try_get<Gpu_material_indices>(model_entity);
        auto material_parameters = Logic_entt().try_get<Gltf_material_parameters>(model_entity);

        // 首先全部设置为零
        if (material_indices != nullptr && !material_indices->empty())
            temp.resize(material_indices->size());

        if (material_indices != nullptr && !material_indices->empty() &&
            material_parameters != nullptr && !material_parameters->empty()) {
            for (const auto &material: *material_parameters) {
                temp.push_back(material_indices->at(material));
            }
        }
        if (!temp.empty())
            set_render_parameter(model_entity, "model_material_parameters", temp);
    }
}


void load_gltf_material_separate(entt::entity model_entity) {
    if (auto path = Logic_entt().try_get<std::filesystem::path>(model_entity)) {
        auto optional_model = get_gltf_model(*path);
        if (optional_model.has_value()) {
            auto &model            = optional_model.value();
            auto &material_indices = Logic_entt().emplace<Gpu_material_indices>(model_entity);
            load_materials(material_indices, *path, model);
        }
    }
    update_material(model_entity);
}


entt::entity load_gltf_model(const std::string &name, const std::filesystem::path &path,
                             const Eigen::Vector3f offset,
                             const Eigen::Quaternionf &rotate,
                             const Eigen::Vector3f zoom) {
    auto optional_model = get_gltf_model(path);
    if (optional_model.has_value()) {
        const entt::entity model_entity = Logic_entt().create();
        Logic_entt().emplace<Name_component>(model_entity, name);
        Logic_entt().emplace<std::filesystem::path>(model_entity, path);
        world_root_add_child(model_entity);
        auto &model = optional_model.value();
        logic_create_proxy(model_entity);
        const auto &transform = Logic_entt().emplace<Transform>(model_entity, offset, rotate);
        // Logic_entt().emplace<Input_Component>(model_entity, model_3d_Event);


        if (model.skins.empty()) {
            Logic_entt().emplace<Shader_data>(model_entity, Engine::instance().get_shader_manager().get_gltf_shader_data());
        } else {
            Logic_entt().emplace<Shader_data>(model_entity, Engine::instance().get_shader_manager().get_skinning_shader_data());
        }
        logic_update_proxy<Shader_data>(model_entity);
        logic_update_proxy<Name_component>(model_entity);
        logic_update_add_tag<opacity_tag>(model_entity);

        const auto nodes_num = model.nodes.size();
        std::vector<entt::entity> nodes_have_deal;
        nodes_have_deal.resize(nodes_num, entt::null);

        for (const auto &scene: model.scenes) {
            for (const auto node_index: scene.nodeIndices) {
                load_node_data(model, nodes_have_deal, node_index, -1, model_entity);
            }
        }
        // 之后呢? 其实完全是可以在这里操作的
        // 那么需要有一个假设,假设 是 按照  深度优先 的 方式进行的 node 的排序
        gltf_load_skin(model, nodes_have_deal, model_entity);
        gltf_load_animal(model, nodes_have_deal, model_entity);


        add_recursion_function_to_children(model_entity, set_transform_dirty);
        add_recursion_function_to_children(model_entity, update_transform_matrix);
        // 为什么要在这里更新? 因为想要确定 精确的 AABB 包围盒的位置

        auto material_parameters = Logic_entt().emplace<Gltf_material_parameters>(model_entity);
        auto &boxes              = Logic_entt().emplace<std::vector<Render_AABB> >(model_entity);
        auto &matrices           = Logic_entt().emplace<std::vector<Transform_Matrix> >(model_entity);
        // 包含不包含 model_entity 的矩阵
        const Eigen::Matrix4f model_entity_matrix = transform.get_transform_matrix();
        Logic_entt().emplace_or_replace<Transform_Matrix>(model_entity, model_entity_matrix);

        auto update_aabb = [&](const entt::entity entity) {
            if (entity != entt::null && Logic_entt().all_of<Geometry_data, Transform_Matrix>(entity)) {
                const auto geometry_data = Logic_entt().get<Geometry_data>(entity);
                const auto aabbs         = geometry_data.get_aabbs();
                const auto &model_matrix = Logic_entt().get<Transform_Matrix>(entity);
                for (auto &bound_box: aabbs) {
                    auto temp = transform_AABB(bound_box, model_matrix);
                    boxes.push_back(temp);
                    matrices.push_back(model_matrix);
                }
            }
        };
        add_recursion_function_to_children(model_entity, update_aabb);


        // 获取 每个 entity 的 全部 primitive 的 包围盒
        auto local_aabb = merge_AABBs(boxes);
        Logic_entt().emplace<Local_Space_AABB>(model_entity, local_aabb);
        const auto world_aabb = transform_AABB(local_aabb, model_entity_matrix);
        Logic_entt().emplace_or_replace<World_Space_AABB>(model_entity, world_aabb.get_aabb_min());


        Geometry_data bindless_Geometry_data;

        auto geometry_function = [&](const entt::entity entity) {
            if (entity != entt::null &&
                Logic_entt().all_of<Geometry_data_need_copy_tag, Geometry_data, Transform_Matrix>(entity)) {
                const auto geometry_data = Logic_entt().get<Geometry_data>(entity);
                const auto vertices      = geometry_data.get_vertices();
                const auto indices       = geometry_data.get_indices();
                const auto materials     = geometry_data.get_materials();
                for (const auto &vertex: vertices) {
                    bindless_Geometry_data.push_vertices(vertex);
                }
                for (const auto &index: indices) {
                    bindless_Geometry_data.push_indices(index);
                }
                for (const auto &material: materials) {
                    material_parameters.push_back(material);
                }
                Render_entt().remove<Geometry_data_need_copy_tag>(entity);
            }
        };
        add_recursion_function_to_children(model_entity, geometry_function);


        auto mesh       = create_mesh_data(bindless_Geometry_data);
        auto primitives = create_primitives(bindless_Geometry_data);

        Logic_entt().emplace<std::vector<VKR_Primitive> >(model_entity, primitives);

        gltf_update_joint_matrix(model_entity);

        update_primitives_model_matrix(model_entity);

        logic_update_proxy(model_entity, boxes);
        logic_update_proxy(model_entity, mesh);
        logic_update_proxy(model_entity, matrices); // 这里给出的是什么? model 本身 不变的? 还是 会变动的呢?

        update_material(model_entity);

        Logic_entt().emplace<GPU_frustum_cull>(model_entity);
        update_primitives_model_box(model_entity);

        return model_entity;
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

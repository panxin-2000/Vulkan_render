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
#include <oneapi/tbb/detail/_task.h>

#include "3d_model_display.h"
#include "camera_optical_component.h"
#include "Command_calculate.h"
#include "scene_component.h"
#include "world_scene_root.h"

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
            number_of_vertices   = current_accessor.count;
        }
    }
    for (const auto &primitive: mesh.primitives) {
        if (primitive.indicesAccessor.has_value()) {
            auto result = copy_indices_data(model, primitive);
            Logic_entt().get_or_emplace<Geometry_data>(entity).push_indices(result);
        } else {
            auto result = create_continue_indices(number_of_vertices);
        }
        auto result = copy_vertices_data(vertices_memory_size, model, primitive);
        Logic_entt().get_or_emplace<Geometry_data>(entity).push_vertices(result);
    }
    // 其实到这里才确定了几何的数据类型
    const auto &data                         = Logic_entt().get_or_emplace<Geometry_data>(entity);
    const std::vector<share_block> &vertices = data.get_vertices();
    auto bound_box                           = find_min_max_point(vertices);
    Logic_entt().emplace_or_replace<AABB_min_max<Point_3> >(entity, bound_box);

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
                            const int current_node_index,
                            const int parent_node_index      = -1,
                            const entt::entity parent_entity = entt::null) {
    auto node                              = model.nodes[current_node_index];
    const entt::entity entity              = Logic_entt().create();
    nodes_have_deal.at(current_node_index) = entity;
    Logic_entt().emplace<Name_component>(entity, node.name.c_str());
    if (parent_entity == entt::null) world_root_add_child(entity);
    else add_relation(parent_entity, entity);
    add_Transform_parameter(entity, node);
    if (node.meshIndex.has_value()) {
        auto material = Logic_entt().get_or_emplace<PBR_component>(entity);
        // set_render_parameter(entity, "object_material", material);
        get_mesh_from_gltf_model(entity, model, node.meshIndex.value());
        // Logic_entt().emplace<Input_Component>(entity, model_3d_Event);
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


struct InverseBindMatrix {
    Eigen::Matrix4f matrix;
};

struct JointMatrix {
    Eigen::Matrix4f matrix;
};


struct RuntimeChannel {
    entt::entity effect_entity   = entt::null;
    fastgltf::AnimationPath path = fastgltf::AnimationPath::Translation;
    std::vector<float> keyframeTimes;
    std::vector<fastgltf::AnimationInterpolation> interpolations;
    std::variant<std::vector<Eigen::Vector3f>, std::vector<Eigen::Quaternionf> > offset_rotate;


    auto get_interpolation_rotation(const float &time, const size_t index,
                                    const std::vector<Eigen::Quaternionf> &rotates) const {
        if (interpolations[index] == fastgltf::AnimationInterpolation::Step) {
            uint32_t last = index;
            return rotates[last];
        } else if (interpolations[index] == fastgltf::AnimationInterpolation::Linear) {
            uint32_t last = index;
            uint32_t next = index + 1;
            float t = (time - keyframeTimes[last]) / (keyframeTimes[next] - keyframeTimes[last]);
            auto last_rotate = rotates[last];
            auto next_rotate = rotates[next];
            Eigen::Quaternionf q_interpolated = last_rotate.slerp(t, next_rotate);
            return q_interpolated;
        }
    }

    size_t get_time_index(const float &time) const {
        for (uint32_t i = 0; i < keyframeTimes.size(); ++i) {
            if (time >= keyframeTimes[i]) {
                return i;
            }
        }
        return 0;
    }

    auto get_interpolation_offset_zoom(const float time, const size_t index,
                                       const std::vector<Eigen::Vector3f> &offsets_or_zooms) const {
        if (interpolations[index] == fastgltf::AnimationInterpolation::Step) {
            uint32_t last = index;
            return offsets_or_zooms[last];
        } else if (interpolations[index] == fastgltf::AnimationInterpolation::Linear) {
            uint32_t last = index;
            uint32_t next = index + 1;
            float t = (time - keyframeTimes[index]) / (keyframeTimes[next] - keyframeTimes[last]);
            auto last_offset = offsets_or_zooms[last];
            auto next_offset = offsets_or_zooms[next];
            Eigen::Vector3f offset_interpolated = (1.0f - t) * last_offset + t * next_offset;
            return offset_interpolated;
        }
        assert("program can run to here " && false);
        return Eigen::Vector3f{1, 1, 1};
    }

    void generate_local_JointTransform(const float time) const {
        auto &value               = Logic_entt().get<Transform>(effect_entity);
        Eigen::Quaternionf rotate = value.get_rotate();
        Eigen::Vector3f offset    = value.get_offset();
        Eigen::Vector3f zoom      = value.get_zoom();
        const auto index          = get_time_index(time);

        switch (path) {
            case fastgltf::AnimationPath::Rotation: {
                if (const auto rotates = std::get_if<std::vector<Eigen::Quaternionf> >(&offset_rotate)) {
                    rotate = get_interpolation_rotation(time, index, *rotates);
                }
                break;
            }
            case fastgltf::AnimationPath::Translation: {
                if (const auto offsets = std::get_if<std::vector<Eigen::Vector3f> >(&offset_rotate)) {
                    auto temp = get_interpolation_offset_zoom(time, index, *offsets);
                    offset    = {temp.x(), temp.y(), temp.z()};
                }
                break;
            }
            case fastgltf::AnimationPath::Scale: {
                if (const auto offsets = std::get_if<std::vector<Eigen::Vector3f> >(&offset_rotate)) {
                    auto temp = get_interpolation_offset_zoom(time, index, *offsets);
                    zoom      = {temp.x(), temp.y(), temp.z()};
                }
                break;
            }
            case fastgltf::AnimationPath::Weights: {
                break;
            }
            default: {
                break;
            }
        }
        Logic_entt().emplace_or_replace<Transform>(effect_entity, offset, rotate, zoom);
        Logic_entt().emplace_or_replace<Transform_matrix_dirty>(effect_entity);
    }
};

struct RuntimeAnimation {
    std::string name;
    float max_frame_time = 0.0f; // 整个动画的总时长（等于所有 channel 中最大的那个 keyframeTimes.back()）
    std::vector<RuntimeChannel> channels;

    void apply_animation(const float time, bool circle_animal = false) const {
        auto number = std::floor(time / max_frame_time);
        if (time > max_frame_time && circle_animal == true) {
            for (const auto &channel: channels) {
                channel.generate_local_JointTransform(time - number * max_frame_time);
            }
            return;
        } else if (time > max_frame_time && circle_animal == false) {
            return;
        }
        for (const auto &channel: channels) {
            channel.generate_local_JointTransform(time);
        }
    }
};

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
            max_frame_time = std::max(max_frame_time, temp_channel.keyframeTimes.back());
            temp_channel.interpolations.reserve(timeAccessor.count);
            temp_channel.interpolations.push_back(sampler.interpolation);

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

void gltf_load_skin(const fastgltf::Asset &model,
                    const std::vector<entt::entity> &nodes_have_deal,
                    const entt::entity &root_entity) {
    for (auto skin: model.skins) {
        if (skin.skeleton.has_value()) {
            size_t rootNodeIdx = skin.skeleton.value();
            std::cout << "  Skeleton Root Node Index: " << rootNodeIdx << "\n";
            // 指向整个骨骼关节层级树（Joints Hierarchy）的公共根节点
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

                std::vector<entt::entity> skin_joints;
                for (auto i = 0; i < skin.joints.size(); ++i) {
                    auto joint  = skin.joints[i];
                    auto entity = nodes_have_deal[joint];
                    Logic_entt().emplace<InverseBindMatrix>(entity, matrixData[i]);
                    skin_joints.push_back(entity);
                }
                Logic_entt().emplace<std::vector<entt::entity> >(root_entity, skin_joints);
            }
        } else {
            // 如果 glTF 没提供 IBM，根据规范，所有关节默认使用单位矩阵 (Identity Matrix)
            std::cout << "  No Inverse Bind Matrices found. Using identity matrices.\n";
        }
    }
}


void update_joint_matrix_matrix(const entt::entity entity) {
    if (Logic_entt().all_of<Transform_Matrix, Scene_Component, InverseBindMatrix>(entity)) {
        auto transform_matrix           = Logic_entt().get<Transform_Matrix>(entity);
        const auto &inverse_bind_matrix = Logic_entt().get<InverseBindMatrix>(entity);
        Eigen::Matrix4f result          = transform_matrix.get() * inverse_bind_matrix.matrix;
        Logic_entt().emplace_or_replace<JointMatrix>(entity, result);
    }
};


entt::entity load_gltf_model(const std::string &name, const std::filesystem::path &path,
                             const Point_3 offset,
                             const Eigen::Quaternionf &rotate,
                             const Point_3 zoom) {
    auto optional_model = get_gltf_model(path);
    if (optional_model.has_value()) {
        const entt::entity model_entity = Logic_entt().create();
        Logic_entt().emplace<Name_component>(model_entity, name);
        world_root_add_child(model_entity);
        auto &model = optional_model.value();

        if (model.skins.empty()) {
            logic_create_proxy(model_entity);
            Logic_entt().emplace<shader_data>(model_entity, Engine::instance().get_gltf_shader_data());
            logic_update_proxy<shader_data>(model_entity);
            logic_update_add_tag<gltf_tag>(model_entity);
        } else {
            logic_create_proxy(model_entity);
            Logic_entt().emplace<shader_data>(model_entity, Engine::instance().get_skinning_shader_data());
            logic_update_proxy<shader_data>(model_entity);
            logic_update_add_tag<skinning_tag>(model_entity);
        }
        logic_update_proxy<Name_component>(model_entity);
        logic_update_add_tag<opacity_tag>(model_entity);

        const auto nodes_num = model.nodes.size();
        std::vector<entt::entity> nodes_have_deal;
        nodes_have_deal.resize(nodes_num, entt::null);
        size_t has_mesh = 0;
        size_t mesh_entity_index;
        for (auto i = 0; i < nodes_num; ++i) {
            auto node = model.nodes[i];
            if (node.meshIndex.has_value()) {
                has_mesh++;
                mesh_entity_index = node.meshIndex.value();
            }
        }
        for (const auto &scene: model.scenes) {
            const entt::entity entity = Logic_entt().create();
            Logic_entt().emplace<Name_component>(entity, scene.name.c_str());
            add_relation(model_entity, entity);
            for (const auto node_index: scene.nodeIndices) {
                load_node_data(model, nodes_have_deal, node_index, -1, entity);
            }
        }
        // 之后呢? 其实完全是可以在这里操作的
        // 那么需要有一个假设,假设 是 按照  深度优先 的 方式进行的 node 的排序
        gltf_load_skin(model, nodes_have_deal, model_entity);
        gltf_load_animal(model, nodes_have_deal, model_entity);

        if (auto animation = Logic_entt().try_get<std::vector<RuntimeAnimation> >(model_entity)) {
            animation->at(0).apply_animation(0.0f);
        }

        const auto &transform       = Logic_entt().get<Transform>(nodes_have_deal[mesh_entity_index]);
        Eigen::Matrix4f mesh_matrix = transform.get_transform_matrix();
        mesh_matrix                 = mesh_matrix.inverse().eval();


        add_recursion_function_to_children(model_entity, update_transform_matrix);

        // 单线程的情况下,下面这个函数是对的
        {
            auto boxes    = std::make_shared<std::vector<Render_AABB> >();
            auto matrices = std::make_shared<std::vector<Transform_Matrix> >();
            Geometry_data bindless_Geometry_data;
            // 主要是下面这一行的问题, 之前的时候 全部 是没有问题,但是现在不行了
            //  model_entity
            for (auto entity: nodes_have_deal) {
                if (entity != entt::null &&
                    Logic_entt().all_of<Geometry_data_need_copy_tag, Geometry_data, Transform_Matrix>(entity)) {
                    // auto view = Logic_entt().view<>();
                    // for (const auto entity: view) {
                    auto geometry_data = Logic_entt().get<Geometry_data>(entity);
                    auto vertices      = geometry_data.get_vertices();
                    auto indices       = geometry_data.get_indices();
                    // 这里其实有一个假设是 vertices.size() == indices.size()
                    auto &model_matrix = Logic_entt().get<Transform_Matrix>(entity);
                    for (auto &vertex: vertices) {
                        bindless_Geometry_data.push_vertices(vertex);
                        const auto bound_box          = find_min_max_point(vertex);
                        Eigen::Vector4f new_centroid  = model_matrix.get() * bound_box.centroid_points;
                        Eigen::Matrix3f R             = model_matrix.get().block<3, 3>(0, 0);
                        Eigen::Vector3f new_direction = R.cwiseAbs() * bound_box.direction_intervals.head<3>();
                        boxes->push_back({
                                             {new_centroid.x(), new_centroid.y(), new_centroid.z(), 1.0f},
                                             {new_direction.x(), new_direction.y(), new_direction.z(), 0.0f}
                                         });
                        matrices->push_back(model_matrix); // 暂时不想太复杂,暂时先放在这里
                    }
                    for (auto &index: indices) {
                        bindless_Geometry_data.push_indices(index);
                    }
                    Render_entt().remove<Geometry_data_need_copy_tag>(entity);
                }
            } {
                auto function = [& mesh_matrix](const entt::entity entity) {
                    if (Logic_entt().all_of<Transform_Matrix, Scene_Component, InverseBindMatrix>(entity)) {
                        auto transform_matrix = Logic_entt().get<Transform_Matrix>(entity);
                        const auto &inverse_bind_matrix = Logic_entt().get<InverseBindMatrix>(entity);
                        Eigen::Matrix4f result = mesh_matrix * transform_matrix.get() * inverse_bind_matrix.matrix;
                        Logic_entt().emplace_or_replace<JointMatrix>(entity, result);
                    }
                };
                add_recursion_function_to_children(model_entity, function);
            }


            // 那么另外一件事 包围盒 应该也是需要去重新计算了
            // 得到全部了,那么需要做什么呢? 上传到一个 buffer 中 生成 一个 std::vector<VKR_Primitive>
            // 多个primitive 连续 才能合并,最后如果可以的话,是可以调用一个 命令来完成的
            auto mesh       = create_mesh_data(bindless_Geometry_data);
            auto primitives = create_primitives(bindless_Geometry_data);
            for (uint32_t i = 0; i < primitives.size(); ++i) {
                primitives.at(i).firstInstance = i;
                // std::cout << "vertexOffset :" << i << std::endl;
                // std::cout << "vertexOffset :" << primitives.at(i).draw_command.indexed_command.vertexOffset << std::endl;
                // std::cout << "firstIndex   :" << primitives.at(i).draw_command.indexed_command.firstIndex << std::endl;
            }
            if (auto skin_joints = Logic_entt().try_get<std::vector<entt::entity> >(model_entity)) {
                std::vector<Eigen::Matrix4f> JointMatrices;
                for (const auto entity: *skin_joints) {
                    JointMatrices.push_back(Logic_entt().get<JointMatrix>(entity).matrix);
                    // 如果有问题, 是上面递归的问题,不会是这里的问题
                }
                const auto matrix_ptr = JointMatrices.data();
                auto matrix_size      = JointMatrices.size() * sizeof(Eigen::Matrix4f);
                auto matrix_buffer    = copy_data_to_gpu_memory(matrix_ptr, matrix_size);
                set_render_parameter(model_entity, "JointMatrices", matrix_buffer);
            }
            // for (uint32_t i = 2000; i < primitives.size(); ++i) {
            //     primitives.at(i) = primitives.at(i - 1000);
            // }
            logic_update_proxy(model_entity, primitives);
            logic_update_proxy(model_entity, boxes);
            logic_update_proxy(model_entity, mesh);
            logic_update_proxy(model_entity, matrices);
            // 现在已经把 model_matrix 全部上传了
            Command_calculate command_calculate;
            command_calculate.command_size = primitives.size(); {
                const auto matrix_ptr = matrices->data();
                auto matrix_size      = matrices->size() * sizeof(Transform_Matrix);
                auto matrix_buffer    = copy_data_to_gpu_memory(matrix_ptr, matrix_size);
                set_render_parameter(model_entity, "model_matrix_parameters", matrix_buffer);
            } {
                const auto boxes_ptr                = boxes->data();
                auto boxes_size                     = boxes->size() * sizeof(Render_AABB);
                auto boxes_buffer                   = copy_data_to_gpu_memory(boxes_ptr, boxes_size);
                command_calculate.AABB_boxesAddress = boxes_buffer->get_gpu_device_address();
                command_calculate.AABB_boxes_buffer = boxes_buffer;
            } {
                const auto primitives_ptr                 = primitives.data();
                auto primitives_size                      = primitives.size() * sizeof(VKR_Primitive);
                auto primitives_buffer                    = copy_data_to_gpu_memory(primitives_ptr, primitives_size);
                command_calculate.IndirectCommandsAddress = primitives_buffer->get_gpu_device_address();
                command_calculate.command_buffer          = primitives_buffer;
            }
            logic_update_proxy(model_entity, command_calculate);


            // 改上传的参数我都已经准备好了 , 只是还没有完全移交到 engine 中


            // 另一个紧接着的问题是  之后呢?
            // entity 的顺序 和上面的顺序是相同的吗? 有必要相同吗?
            // 这里是单个 还是可以的,但是多个的时候呢?
            // 该算的应该已经算的差不多了,之后就是如何上传的问题了
            // 之后就是应该怎么做呢?
            // boxes 还是需要上传的, primitives 需要选择一个方式然后上传
            // 之后就应该交由 渲染线程 来进行 更新结果了 然后看看怎么用一个参数完成调用  material  还是需要 选一个位置的
            // 然后这里才是合并为一个 entity 看看是否需要去 传递给 render_thread , 当然,这里也还只是暂时的,
        }

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

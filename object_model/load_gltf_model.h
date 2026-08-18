//
// Created by 潘鑫 on 2026/3/16.
//

#ifndef HELLO_MAC_LOAD_GLTF_MODEL_H
#define HELLO_MAC_LOAD_GLTF_MODEL_H

#include "global_singleton.h"
#include "base_geometry/base.h"
#include <Eigen/Eigen>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include "scene_component.h"
#include "transform_component.h"


struct InverseBindMatrix {
    Eigen::Matrix4f matrix;
};

struct Inverse_Global_Transform {
    Eigen::Matrix4f matrix;
};

struct JointMatrix {
    Eigen::Matrix4f matrix;
};

struct JointMatrixDirty {
};

struct ModelMatricesDirty {
};


#include <Eigen/Dense>

inline Eigen::Vector3f InterpolateCubicSplineVector(
    float s,
    const Eigen::Vector3f &v_k,        // Current Value
    const Eigen::Vector3f &out_tan_k,  // Current OutTangent
    const Eigen::Vector3f &in_tan_kp1, // Next InTangent
    const Eigen::Vector3f &v_kp1,      // Next Value
    float deltaTime = 1.0f
) {
    float s2 = s * s;
    float s3 = s2 * s;

    // 混合系数
    float h00 = 2 * s3 - 3 * s2 + 1;
    float h10 = s3 - 2 * s2 + s;
    float h01 = -2 * s3 + 3 * s2;
    float h11 = s3 - s2;

    // 关键点：切线必须乘以 deltaTime
    Eigen::Vector3f b_k   = deltaTime * out_tan_k;
    Eigen::Vector3f a_kp1 = deltaTime * in_tan_kp1;

    // 埃尔米特插值
    return h00 * v_k + h10 * b_k + h01 * v_kp1 + h11 * a_kp1;
}

#include <Eigen/Dense>

inline Eigen::Quaternionf InterpolateCubicSplineQuaternion(
    float s,
    const Eigen::Quaternionf &v_k,
    const Eigen::Quaternionf &out_tan_k,
    const Eigen::Quaternionf &in_tan_kp1,
    const Eigen::Quaternionf &v_kp1,
    float deltaTime = 1.0f
) {
    float s2 = s * s;
    float s3 = s2 * s;

    float h00 = 2 * s3 - 3 * s2 + 1;
    float h10 = s3 - 2 * s2 + s;
    float h01 = -2 * s3 + 3 * s2;
    float h11 = s3 - s2;

    // 将四元数视为 4D 向量进行计算
    Eigen::Vector4f q0    = v_k.coeffs();
    Eigen::Vector4f out_t = out_tan_k.coeffs();
    Eigen::Vector4f in_t  = in_tan_kp1.coeffs();
    Eigen::Vector4f q1    = v_kp1.coeffs();

    Eigen::Vector4f b_k   = deltaTime * out_t;
    Eigen::Vector4f a_kp1 = deltaTime * in_t;

    // 混合 4D 向量
    Eigen::Vector4f result_vec = h00 * q0 + h10 * b_k + h01 * q1 + h11 * a_kp1;

    // 还原为四元数并【必须】归一化
    Eigen::Quaternionf result_q(result_vec[3], result_vec[0], result_vec[1], result_vec[2]); // w, x, y, z
    result_q.normalize();

    return result_q;
}


struct RuntimeChannel {
    entt::entity effect_entity   = entt::null;
    fastgltf::AnimationPath path = fastgltf::AnimationPath::Translation;
    std::vector<float> keyframeTimes;
    fastgltf::AnimationInterpolation interpolation;
    std::variant<std::vector<Eigen::Vector3f>, std::vector<Eigen::Quaternionf> > offset_rotate;


    [[nodiscard]] auto get_interpolation_rotation(const float &time, const size_t index,
                                                  const std::vector<Eigen::Quaternionf> &rotates) const {
        if (interpolation == fastgltf::AnimationInterpolation::Step) {
            const uint32_t last = index;
            return rotates[last];
        } else if (interpolation == fastgltf::AnimationInterpolation::Linear) {
            const uint32_t last = index;
            const uint32_t next = index + 1;
            const float t = (time - keyframeTimes[last]) / (keyframeTimes[next] - keyframeTimes[last]);
            const auto last_rotate = rotates[last];
            const auto next_rotate = rotates[next];
            Eigen::Quaternionf q_interpolated = last_rotate.slerp(t, next_rotate);
            q_interpolated.normalize();
            return q_interpolated;
        } else if (interpolation == fastgltf::AnimationInterpolation::CubicSpline) {
        }
    }

    [[nodiscard]] size_t get_time_index(const float &time) const {
        for (uint32_t i = 0; i < keyframeTimes.size() - 1; ++i) {
            if (time >= keyframeTimes[i] && time <= keyframeTimes[i + 1]) {
                return i;
            }
        }
        return 0;
    }

    auto get_interpolation_offset_zoom(const float time, const size_t index,
                                       const std::vector<Eigen::Vector3f> &offsets_or_zooms) const {
        if (interpolation == fastgltf::AnimationInterpolation::Step) {
            uint32_t last = index;
            return offsets_or_zooms[last];
        } else if (interpolation == fastgltf::AnimationInterpolation::Linear) {
            uint32_t last = index;
            uint32_t next = index + 1;
            float t = (time - keyframeTimes[index]) / (keyframeTimes[next] - keyframeTimes[last]);
            auto last_offset = offsets_or_zooms[last];
            auto next_offset = offsets_or_zooms[next];
            Eigen::Vector3f offset_interpolated = (1.0f - t) * last_offset + t * next_offset;
            return offset_interpolated;
        } else if (interpolation == fastgltf::AnimationInterpolation::CubicSpline) {
            // 这里还是稍微有点麻烦的
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
                // todo:
                break;
            }
            default: {
                break;
            }
        }
        Logic_entt().emplace_or_replace<Transform>(effect_entity, offset, rotate, zoom);
        Logic_entt().emplace_or_replace<Transform_matrix_dirty>(effect_entity);
        add_recursion_function_to_children(effect_entity, set_transform_dirty);
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

/**
 * 这里的存储的是已经 上传到GPU 上的 material 的索引
 */
class Gpu_material_indices : public std::vector<uint32_t> {
public:
    using std::vector<uint32_t>::vector;
};

/**
 * 这里存储的 gltf 的 assert 中 的 material 的索引, 需要通过上一个转化为 engine 的才能上传到GPU
 */
class Gltf_material_parameters : public std::vector<uint32_t> {
public:
    using std::vector<uint32_t>::vector;
};


void load_gltf_material_separate(entt::entity model_entity);

void update_material(entt::entity model_entity);

entt::entity load_gltf_model(const std::string &name, const std::filesystem::path &mesh_path,
                             const Eigen::Vector3f            = {0, 0, 0},
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity(),
                             const Eigen::Vector3f            = {1, 1, 1});

void gltf_update_joint_matrix(const entt::entity &model_entity);

#endif //HELLO_MAC_LOAD_GLTF_MODEL_H

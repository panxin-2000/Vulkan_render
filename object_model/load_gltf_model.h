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


struct RuntimeChannel {
    entt::entity effect_entity   = entt::null;
    fastgltf::AnimationPath path = fastgltf::AnimationPath::Translation;
    std::vector<float> keyframeTimes;
    std::vector<fastgltf::AnimationInterpolation> interpolations;
    std::variant<std::vector<Eigen::Vector3f>, std::vector<Eigen::Quaternionf> > offset_rotate;


    [[nodiscard]] auto get_interpolation_rotation(const float &time, const size_t index,
                                                  const std::vector<Eigen::Quaternionf> &rotates) const {
        if (interpolations[index] == fastgltf::AnimationInterpolation::Step) {
            const uint32_t last = index;
            return rotates[last];
        } else if (interpolations[index] == fastgltf::AnimationInterpolation::Linear) {
            const uint32_t last = index;
            const uint32_t next = index + 1;
            const float t = (time - keyframeTimes[last]) / (keyframeTimes[next] - keyframeTimes[last]);
            const auto last_rotate = rotates[last];
            const auto next_rotate = rotates[next];
            Eigen::Quaternionf q_interpolated = last_rotate.slerp(t, next_rotate);
            return q_interpolated;
        }
    }

    [[nodiscard]] size_t get_time_index(const float &time) const {
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
                // todo:
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


entt::entity load_gltf_model(const std::string &name, const std::filesystem::path &mesh_path,
                             const Point_3 offset             = Point_3(0, 0, 0),
                             const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity(),
                             const Point_3 zoom               = Point_3(1, 1, 1));

void gltf_update_joint_matrix(const entt::entity &model_entity);

#endif //HELLO_MAC_LOAD_GLTF_MODEL_H

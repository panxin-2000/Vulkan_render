//
// Created by 潘鑫 on 2026/3/8.
//


#include "load_gltf_model.h"
#include "model_matrix.h"
#include "transform_component.h"
#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_render_manage.h"
#include "world_scene_root.h"


inline void update_object_transform_function(float time_milliseconds) { {
        const auto view = Logic_entt().view<UI_transform_dirty, Proxy_entity, Rect_2D_transform>();
        // 包围盒发生了更新
        for (const auto it: view) {
            auto pos    = view.get<Rect_2D_transform>(it);
            auto offset = pos.get_offset();
            matrix_4x4 model_matrix;
            UI_matrix_4x4(&model_matrix, {1, 1}, pos.get_offset());
            set_render_parameter(it, "model_4x4", model_matrix); // 这里直接设置有问题，到渲染线程之后再设置
            Logic_entt().remove<UI_transform_dirty>(it);
        }
    } {
        auto root = get_world_root();
        // 逻辑大概是这个样子，但是实际的细节，还是有优化的地方的
        // 第一个是 没有 Transform 的时候，其实应该默认 给出单位矩阵
        // 如果中间存在一个没有的时候，需要添加一个判断，是否需要向下传递，
        // 目前是从零开始把全部的节点都遍历了一遍

        {
            const auto view = Logic_entt().view<std::vector<RuntimeAnimation> >();
            for (const auto entity: view) {
                if (auto animation = Logic_entt().try_get<std::vector<RuntimeAnimation> >(entity)) {
                    // 怎么把下面这个 给到一个 时间线呢?
                    animation->at(0).apply_animation(time_milliseconds, true);
                    Logic_entt().emplace_or_replace<JointMatrixDirty>(entity);
                }
            }
        }

        add_recursion_function_to_children(root, update_transform_matrix); {
            const auto view = Logic_entt().view<JointMatrixDirty>();
            for (const auto it: view) {
                gltf_update_joint_matrix(it);
                Logic_entt().remove<JointMatrixDirty>(it);
            }
        }
    }
}


void sync_render_data_to_render_thread(float time_milliseconds) {
    // 应该不止更新 position，还有很多的都需要更新
    // 其实下面的两个也不应该这样写
    update_camera_transform();
    update_object_transform_function(time_milliseconds);

    // 中间这部分需要移动
    Command_submit_manager::set_sync();
    vk_render_queue::instance().logic_add_finished();
}


bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = Logic_entt().try_get<Proxy_entity>(entity)) {
        const auto entity_temp = render->entity_;
        auto lambda            = [entity_temp]() {
            Render_entt().emplace_or_replace<Render_destroy_tag>(entity_temp);
        };
        vk_render_queue::instance().render_update_entt(lambda);
        return true;
    }
    return false;
}

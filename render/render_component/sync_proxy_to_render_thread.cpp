//
// Created by 潘鑫 on 2026/3/8.
//


#include "model_matrix.h"
#include "transform_component.h"
#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_render_manage.h"
#include "world_scene_root.h"


inline void update_object_transform_function() { {
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
        auto function = [](const entt::entity entity) {
            if (Logic_entt().all_of<Transform, Scene_Component, Transform_matrix_dirty>(entity)) {
                // 满足条件：两个组件都有
                auto parent_entity           = get_parent(entity);
                auto parent_transform_matrix = Logic_entt().get_or_emplace<Transform_matrix>(parent_entity);
                const auto &transform        = Logic_entt().get<Transform>(entity);
                Eigen::Matrix4f result       = parent_transform_matrix.get() * transform.get_transform_matrix();
                Logic_entt().emplace_or_replace<Transform_matrix>(entity, result);
                if (Logic_entt().all_of<Proxy_entity, Transform_matrix>(entity)) {
                    set_render_parameter(entity, "model_4x4", result);
                }
                if (Logic_entt().all_of<Proxy_entity, AABB_min_max<Point_3> >(entity)) {
                    const auto &aabb = Logic_entt().get<AABB_min_max<Point_3> >(entity);
                    const AABB_centroid<Point_3> aabb_centroid(aabb);
                    const AABB_min_max<Point_3> new_aabb = aabb_centroid.multiply_matrix(result);
                    logic_update_proxy(entity, new_aabb); //
                }
                Logic_entt().remove<Transform_matrix_dirty>(entity);
            }
        };
        // 目前是从零开始把全部的节点都遍历了一遍
        add_recursion_function_to_children(root, function);
    }
}


void sync_render_data_to_render_thread() {
    // 应该不止更新 position，还有很多的都需要更新
    // 其实下面的两个也不应该这样写
    update_camera_transform();
    update_object_transform_function();

    // 中间这部分需要移动


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

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
                Logic_entt().remove<Transform_matrix_dirty>(entity);
            }
        };
        // 目前是从零开始把全部的节点都遍历了一遍
        add_recursion_function_to_children(root, function);
    } {
        std::vector<Render_AABB> boxes;
        std::vector<Transform_matrix> matrices;
        Geometry_data bindless_Geometry_data;
        auto view = Render_entt().view<Geometry_data_need_copy_tag, Geometry_data, Transform_matrix>();
        for (const auto entity: view) {
            auto geometry_data = Render_entt().get<Geometry_data>(entity);
            auto vertices      = geometry_data.get_vertices();
            auto indices       = geometry_data.get_indices();
            // 这里其实有一个假设是 vertices.size() == indices.size()
            auto &model_matrix = Render_entt().get<Transform_matrix>(entity);
            for (auto &vertex: vertices) {
                bindless_Geometry_data.push_vertices(vertex);
                const auto bound_box          = find_min_max_point(vertex);
                Eigen::Vector4f new_centroid  = model_matrix.get() * bound_box.centroid_points;
                Eigen::Matrix3f R             = model_matrix.get().block<3, 3>(0, 0);
                Eigen::Vector3f new_direction = R.cwiseAbs() * bound_box.direction_intervals.head<3>();
                boxes.push_back({
                                    {new_centroid.x(), new_centroid.y(), new_centroid.z(), 1.0f},
                                    {new_direction.x(), new_direction.y(), new_direction.z(), 0.0f}
                                });
                matrices.push_back(model_matrix); // 暂时不想太复杂,暂时先放在这里
            }
            for (auto &index: indices) {
                bindless_Geometry_data.push_indices(index);
            }
            Render_entt().remove<Geometry_data_need_copy_tag>(entity);
        }
        // 那么另外一件事 包围盒 应该也是需要去重新计算了
        // 得到全部了,那么需要做什么呢? 上传到一个 buffer 中 生成 一个 std::vector<VKR_Primitive>
        // 多个primitive 连续 才能合并,最后如果可以的话,是可以调用一个 命令来完成的
        auto mesh       = create_mesh_data(bindless_Geometry_data);
        auto primitives = create_primitives(bindless_Geometry_data);
        // 另一个紧接着的问题是  之后呢?
        // entity 的顺序 和上面的顺序是相同的吗? 有必要相同吗?
        // 这里是单个 还是可以的,但是多个的时候呢?
        // 该算的应该已经算的差不多了,之后就是如何上传的问题了
        // 之后就是应该怎么做呢?
        // boxes 还是需要上传的, primitives 需要选择一个方式然后上传
        // 之后就应该交由 渲染线程 来进行 更新结果了 然后看看怎么用一个参数完成调用  material  还是需要 选一个位置的
        // 然后这里才是合并为一个 entity 看看是否需要去 传递给 render_thread , 当然,这里也还只是暂时的,
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

//
// Created by 潘鑫 on 2026/2/17.
//

#ifndef HELLO_MAC_UI_POSITION_AND_OFFSET_H
#define HELLO_MAC_UI_POSITION_AND_OFFSET_H
#include <scene_component.h>
#include "name_component.h"

class Rect_transform {
    Point_2 zoom   = {1, 1};
    Point_2 offset = {0, 0};

    AABB_centroid<Point_2> bounding_box_; // 每次都直接计算吧。

public:
    [[nodiscard]] Point_2 get_zoom() const {
        return zoom;
    }

    [[nodiscard]] Point_2 get_offset() const {
        return offset;
    }


    void set_bounding_box(const Point_2 min, const Point_2 max) {
        bounding_box_ = AABB_centroid<Point_2>(min, max);
    }

    AABB_min_max<Point_2> get_bounding_box() {
        return bounding_box_;
    }

    bool set_zoom(const entt::entity entity, const base_event_with_stamp &base_event) {
        zoom.x = zoom.x * std::powf(1.5, base_event.scroll.x * 0.01);
        zoom.y = zoom.y * std::powf(1.5, base_event.scroll.y * 0.01);
        g_entt().emplace_or_replace<Position_update_tag>(entity);
        // if (auto *scene_node = g_entt().try_get<Scene_Component>(entity)) {
        //     for (const entt::entity children_entity: scene_node->children) {
        //         if (g_entt().valid(children_entity)) {
        //             g_entt().emplace_or_replace<Position_update_tag>(children_entity);
        //         }
        //     }
        // }
        return true;
    }

    bool set_position_offset(const entt::entity entity, const base_event_with_stamp &base_event) {
        // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
        // 更改坐标系的范围，x轴是从左到右，范围是-1到1之间，y轴是从下到上，范围是-1到1之间
        Point_2 move                 = base_event.current_position - base_event.last_position;
        bounding_box_.centroid_point = bounding_box_.centroid_point + move;
        offset                       = offset + move;
        g_entt().emplace_or_replace<Position_update_tag>(entity);
        // if (auto *scene_node = g_entt().try_get<Scene_Component>(entity)) {
        //     for (const entt::entity children_entity: scene_node->children) {
        //         if (g_entt().valid(children_entity)) {
        //             g_entt().emplace_or_replace<Position_update_tag>(children_entity);
        //         }
        //     }
        // }
        // std::cout << "move x: " << offset.x << " y: " << offset.y << std::endl;
        // offset.x = offset.x + move.x / get_win_WIDTH() * 2;
        // offset.y = offset.y - move.y / get_win_HEIGHT() * 2; // todo: 检查为什么要反y轴，有没有办法只改一个参数
        return true;
    }

    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = g_entt().try_get<Rect_transform>(entity)) {
            if (intersect(scene_node->bounding_box_, current_position)) {
                return true;
            }
        }
        return false;
    }

    static bool check_entity_children_intersect_point(std::vector<entt::entity> *return_value,
                                                      entt::entity entity,
                                                      const Point_2 &current_position) {
        if (auto *scene_node = g_entt().try_get<Scene_Component>(entity)) {
            for (const entt::entity children_entity: scene_node->children) {
                if (check_entity_intersect_point(children_entity, current_position)) {
                    return_value->push_back(children_entity);
                    check_entity_children_intersect_point(return_value, children_entity, current_position);
                }
            }
        }
        return false;
    }

    bool update_2D_position_matrix() const {
        const auto &storage = g_entt().storage<Rect_transform>();
        const auto entity   = entt::to_entity(storage, *this);
        if (auto render = g_entt().try_get<Geometry_data>(entity)) {
        }
        return true;
    }
};


// 回调函数

inline void update_UI_position() {
    // 应该不止更新 position，还有很多的都需要更新
    {
        // 就是检查一下，已经给过 渲染线程，就添加一个 lambda 更新部分内容就好
        // global 相关的内容尽量只能偏移，
        const auto view = g_entt().view<global_uniform_buffer_update>();
        for (const auto &it: view) {
            update_global_bindings_to_descriptor_sets(it);
            g_entt().remove<global_uniform_buffer_update>(it);
        }
    } {
        const auto view = g_entt().view<uniform_buffer_update>();
        for (const auto &it: view) {
            update_object_bindings_to_descriptor_sets(it);
            g_entt().remove<uniform_buffer_update>(it);
        }
    } {
        const auto view = g_entt().view<need_render_tag>(entt::exclude<std::shared_ptr<VKR_object_proxy> >);
        for (const auto &it: view) {
            create_VKR_object_proxy(it); // 因为这里没有区分。全部都在场景的根节点之下
        }
    }
    const auto view = g_entt().view<Position_update_tag, Rect_transform, std::shared_ptr<VKR_object_proxy> >();
    // 位置发生了更新，需要讲更新传递出去
    for (const auto it: view) {
        // get_model_matrix();
        g_entt().remove<Position_update_tag>(it);
        auto pos    = view.get<Rect_transform>(it);
        auto offset = pos.get_offset();
        LOG_INFO(g_log(), "offset x {} y {}", offset.x, offset.y);

        add_geometry_data(it, pos.get_bounding_box().min_point.x,
                          pos.get_bounding_box().min_point.y,
                          pos.get_bounding_box().max_point.x,
                          pos.get_bounding_box().max_point.y);

        const auto mesh = create_mesh(it);

        auto lambda = [mesh](const std::shared_ptr<VKR_object_proxy> &proxy) {
            if (mesh.has_value()) {
                proxy->mesh = mesh.value();;
            } else {
                LOG_INFO(g_log(), "descriptor_sets empty");
            }
        };

        update_VKR_object_proxy(it, lambda);
    }
}


class UI_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = g_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           g_entt().emplace<Scene_Component>(instance);
                           g_entt().emplace<Name_component>(instance, "scene_root");
                           g_entt().emplace<VKR_shader_paths>(instance,
                                                              "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                                              "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                                              "", "");
                           matrix_4x4 view;
                           UI_matrix_4x4(&view, 1280, 720);
                           set_render_parameter(instance, "global_view_4x4", view);

                           if (auto *scene_node = g_entt().try_get<Rect_transform>(instance)) {
                               scene_node->set_bounding_box({0, 0},
                                                            {
                                                                static_cast<float>(get_win_WIDTH()),
                                                                static_cast<float>(get_win_HEIGHT())
                                                            });
                           }
                           // 在系统初始化时，给logic_render_data * 的类型都添加这个销毁前执行的函数
                           // g_entt().on_destroy<logic_render_data>().connect<&cleanup_logic_render_data>();
                           // 也可以在只移除 logic_render_data 时 触发，但是不同类型触发的顺序可能是随机的。
                       }
                      );

        return instance;
    }

private
:
    UI_scene_root() = default; // 禁用构造
};


static entt::entity &get_UI_scene_root() {
    return UI_scene_root::get();
}

inline std::vector<entt::entity> UI_stack_intersect(const Point_2 &current_position) {
    std::vector<entt::entity> return_value;
    const auto scene_root_node = get_UI_scene_root();
    return_value.push_back(scene_root_node);
    Rect_transform::check_entity_children_intersect_point(&return_value, scene_root_node, current_position);
    return return_value;
}
#endif //HELLO_MAC_UI_POSITION_AND_OFFSET_H

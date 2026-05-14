//
// Created by 潘鑫 on 2026/2/17.
//

#ifndef HELLO_MAC_UI_POSITION_AND_OFFSET_H
#define HELLO_MAC_UI_POSITION_AND_OFFSET_H
#include <scene_component.h>

#include "descriptor_pool.h"
#include "model_transform_component.h"
#include "name_component.h"
#include "VKR_proxy_component.h"
#include "shader_component.h"
#include "base_geometry/intersect_function.h"

class Rect_2D_transform {
    Point_2 zoom_   = {1, 1};
    Point_2 offset_ = {0, 0};

    AABB_centroid<Point_2> bounding_box_; // 每次都直接计算吧。

public:
    [[nodiscard]] Point_2 get_zoom() const {
        return zoom_;
    }

    [[nodiscard]] Point_2 get_offset() const {
        return offset_;
    }


    void set_bounding_box(const Point_2 min, const Point_2 max) {
        bounding_box_ = AABB_centroid<Point_2>(min, max);
    }

    AABB_min_max<Point_2> get_bounding_box() {
        return bounding_box_;
    }

    bool set_zoom(const entt::entity entity, const base_event_with_stamp &base_event) {
        zoom_.x = zoom_.x * std::powf(1.5, base_event.scroll.x * 0.01);
        zoom_.y = zoom_.y * std::powf(1.5, base_event.scroll.y * 0.01);
        Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
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
        const Point_2 move            = base_event.current_position - base_event.last_position;
        bounding_box_.add_offset(move);
        offset_ = offset_ + move;
        Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
        return true;
    }

    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
            if (intersect(scene_node->bounding_box_, current_position)) {
                return true;
            }
        }
        return false;
    }

    static bool check_entity_children_intersect_point(std::vector<entt::entity> *return_value,
                                                      entt::entity entity,
                                                      const Point_2 &current_position) {
        if (auto *scene_node = Logic_entt().try_get<Scene_Component>(entity)) {
            for (const entt::entity children_entity: scene_node->children_) {
                if (check_entity_intersect_point(children_entity, current_position)) {
                    return_value->push_back(children_entity);
                    check_entity_children_intersect_point(return_value, children_entity, current_position);
                }
            }
        }
        return false;
    }

    bool update_2D_position_matrix() const {
        const auto &storage = Logic_entt().storage<Rect_2D_transform>();
        const auto entity   = entt::to_entity(storage, *this);
        if (auto render = Logic_entt().try_get<Geometry_data>(entity)) {
        }
        return true;
    }
};

inline void update_object_transform_function() { {
        const auto view = Logic_entt().view<UI_transform_dirty, Proxy_entity, Rect_2D_transform>();
        // 包围盒发生了更新
        for (const auto it: view) {
            auto pos    = view.get<Rect_2D_transform>(it);
            auto offset = pos.get_offset();
            matrix_4x4 view;
            UI_matrix_4x4(&view, {1, 1}, pos.get_offset());
            set_render_parameter(it, "model_4x4", view); // 这里直接设置有问题，到渲染线程之后再设置
            Logic_entt().remove<UI_transform_dirty>(it);
        }
    } {
        const auto view = Logic_entt().view<UI_transform_dirty, Proxy_entity, model_transform>();
        for (const auto it: view) {
            auto &transform  = view.get<model_transform>(it);
            auto modelMatrix = transform.update_model_matrix();
            set_render_parameter(it, "model_4x4", modelMatrix);
            Logic_entt().remove<UI_transform_dirty>(it);
        }
    }
}


class UI_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = Logic_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           Logic_entt().emplace<Scene_Component>(instance);
                           Logic_entt().emplace<Name_component>(instance, "scene_root");
                           Logic_entt().emplace<VKR_shader_paths>(instance,
                                                                  "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                                                  "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                                                  "", "");
                           matrix_4x4 view;
                           identity_matrix_4x4(&view);
                           set_render_parameter(instance, "global_view_4x4", view);

                           matrix_4x4 projection;
                           UI_projection_4x4(&projection, 1280, 720);
                           set_render_parameter(instance, "global_projection_4x4", projection);

                           if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(instance)) {
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
    Rect_2D_transform::check_entity_children_intersect_point(&return_value, scene_root_node, current_position);
    return return_value;
}
#endif //HELLO_MAC_UI_POSITION_AND_OFFSET_H

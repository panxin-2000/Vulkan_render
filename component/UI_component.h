//
// Created by 潘鑫 on 2026/2/17.
//

#ifndef HELLO_MAC_UI_POSITION_AND_OFFSET_H
#define HELLO_MAC_UI_POSITION_AND_OFFSET_H
#include <scene_component.h>
#include "name_component.h"

class rect_transform {
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

    bool set_zoom(const entt::entity entity, const base_event_with_stamp &base_event) {
        zoom.x = zoom.x * std::powf(1.5, base_event.scroll.x * 0.01);
        zoom.y = zoom.y * std::powf(1.5, base_event.scroll.y * 0.01);
        g_entt().emplace_or_replace<Position_update_tag>(entity);
        if (auto *scene_node = g_entt().try_get<Scene_Component>(entity)) {
            for (const entt::entity children_entity: scene_node->children) {
                if (g_entt().valid(children_entity)) {
                    g_entt().emplace_or_replace<Position_update_tag>(children_entity);
                }
            }
        }
        return true;
    }

    bool set_position_offset(const entt::entity entity, const base_event_with_stamp &base_event) {
        // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
        // 更改坐标系的范围，x轴是从左到右，范围是-1到1之间，y轴是从下到上，范围是-1到1之间
        Point_2 move                 = base_event.current_position - base_event.last_position;
        bounding_box_.centroid_point = bounding_box_.centroid_point + move;
        offset                       = offset + move;
        g_entt().emplace_or_replace<Position_update_tag>(entity);
        if (auto *scene_node = g_entt().try_get<Scene_Component>(entity)) {
            for (const entt::entity children_entity: scene_node->children) {
                if (g_entt().valid(children_entity)) {
                    g_entt().emplace_or_replace<Position_update_tag>(children_entity);
                }
            }
        }
        // std::cout << "move x: " << offset.x << " y: " << offset.y << std::endl;
        // offset.x = offset.x + move.x / get_win_WIDTH() * 2;
        // offset.y = offset.y - move.y / get_win_HEIGHT() * 2; // todo: 检查为什么要反y轴，有没有办法只改一个参数
        return true;
    }

    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = g_entt().try_get<rect_transform>(entity)) {
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
        const auto &storage = g_entt().storage<rect_transform>();
        const auto entity   = entt::to_entity(storage, *this);
        if (auto render = g_entt().try_get<logic_render_data *>(entity)) {
        }
        return true;
    }
};


// 回调函数


class UI_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = g_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           g_entt().emplace<Scene_Component>(instance);
                           g_entt().emplace<Name_component>(instance, "scene_root");
                           if (auto *scene_node = g_entt().try_get<rect_transform>(instance)) {
                               scene_node->set_bounding_box({0, 0},
                                                            {
                                                                static_cast<float>(get_win_WIDTH()),
                                                                static_cast<float>(get_win_HEIGHT())
                                                            });
                           }
                           // 在系统初始化时，给logic_render_data * 的类型都添加这个销毁前执行的函数
                           // g_entt().on_destroy<logic_render_data *>().connect<&cleanup_logic_render_data>();
                           // 也可以在只移除 logic_render_data * 时 触发，但是不同类型触发的顺序可能是随机的。
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
    rect_transform::check_entity_children_intersect_point(&return_value, scene_root_node, current_position);
    return return_value;
}
#endif //HELLO_MAC_UI_POSITION_AND_OFFSET_H

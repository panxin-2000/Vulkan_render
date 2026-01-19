//
// Created by 潘鑫 on 2025/12/25.
//

#ifndef HELLO_MAC_SCENE_COMPONENT_H
#define HELLO_MAC_SCENE_COMPONENT_H
#include <iostream>
#include <vector>
#include <entt/entt.hpp>
#include "base_event.h"
#include "ECS.h"
#include "base_element/point_3.h"
#include "shader.h"
#include "base_element/intersect/objects_intersect_with_point.h"

class Scene_Component {
private:
    std::vector<entt::entity> parent;
    Point_2 zoom = {1, 1};
    Point_2 offset = {0, 0};

public:
    void set_bounding_box(Point_2 min, Point_2 max) {
        bounding_box_ = AABB_centroid<Point_2>(min, max);
    }

    std::vector<entt::entity> children;

    AABB_centroid<Point_2> bounding_box_; // 每次都直接计算吧。

    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(entity)) {
            if (intersect(scene_node->bounding_box_, current_position)) {
                return true;
            }
        }
        return false;
    }

    static bool check_entity_children_intersect_point(std::vector<entt::entity> *return_value,
                                                      entt::entity entity,
                                                      const Point_2 &current_position) {
        if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(entity)) {
            for (const entt::entity children_entity: scene_node->children) {
                if (check_entity_intersect_point(children_entity, current_position)) {
                    return_value->push_back(children_entity);
                    check_entity_children_intersect_point(return_value, children_entity, current_position);
                }
            }
        }
        return false;
    }

    void add_parent(entt::entity entity) {
        parent.push_back(entity);
    }

    void remove_parent(entt::entity entity) {
        parent.erase(std::remove(parent.begin(), parent.end(), entity), parent.end());
    }

    void remove_children(entt::entity entity) {
        parent.erase(std::remove(parent.begin(), parent.end(), entity), parent.end());
    }

    void add_child(entt::entity entity) {
        children.push_back(entity);
    }

    Point_2 get_zoom() const {
        return zoom;
    }

    Point_2 get_offset() const {
        return offset;
    }

    bool update_2D_position_matrix() {
        const auto &storage = get_entt_instance().storage<Scene_Component>();
        const auto entity = entt::to_entity(storage, *this);
        if (auto *render = get_entt_instance().try_get<logic_render_data>(entity)) {
            data_value_or_ptr data{};
            Shader_object::set_model_transform_zoom_rotate(data.vec_4,
                                                           // {0.01f , 0.01f, 1.0},
                                                           {2.0f / get_win_WIDTH(), 2.0f / get_win_HEIGHT(), 1.0},
                                                           {0.0f, 0.0f, 0.0f},
                                                           {
                                                               ((offset.x / get_win_WIDTH()) - 0.5f) * 2,
                                                               ((offset.y / get_win_HEIGHT()) - 0.5f) * -2,
                                                               0
                                                           });
            render->add_uniform("model_transform", gl_mat4, data);
        }
        return true;
    }

    bool update_position() {
        const auto &storage = get_entt_instance().storage<Scene_Component>();


        const auto entity = entt::to_entity(storage, *this);
        if (auto *render = get_entt_instance().try_get<logic_render_data>(entity)) {
            data_value_or_ptr data{};
            Shader_object::set_model_transform_zoom_rotate(data.vec_4,
                                                           {zoom.x, zoom.y, 1.0},
                                                           {0.0f, 0.0f, 0.0f}, {offset});
            render->add_uniform("model_transform", gl_mat4, data);
        }
        return true;
    }

    bool set_zoom(const base_event_with_stamp &base_event) {
        zoom.x = zoom.x * std::powf(1.5, base_event.scroll.x * 0.01);
        zoom.y = zoom.y * std::powf(1.5, base_event.scroll.y * 0.01);
    }

    bool set_position_offset(const base_event_with_stamp &base_event) {
        // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
        // 更改坐标系的范围，x轴是从左到右，范围是-1到1之间，y轴是从下到上，范围是-1到1之间
        Point_2 move = base_event.current_position - base_event.last_position;
        bounding_box_.centroid_point = bounding_box_.centroid_point + move;
        offset = offset + move;
        // std::cout << "move x: " << offset.x << " y: " << offset.y << std::endl;
        // offset.x = offset.x + move.x / get_win_WIDTH() * 2;
        // offset.y = offset.y - move.y / get_win_HEIGHT() * 2; // todo: 检查为什么要反y轴，有没有办法只改一个参数
        return true;
    }
};


class scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = get_entt_instance().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           get_entt_instance().emplace<Scene_Component>(instance);
                           get_entt_instance().emplace<Name_component>(instance, "scene_root");
                           if (auto *scene_node = get_entt_instance().try_get<Scene_Component>(instance)) {
                               scene_node->set_bounding_box({0, 0},
                                                            {
                                                                static_cast<float>(get_win_WIDTH()),
                                                                static_cast<float>(get_win_HEIGHT())
                                                            });
                           }
                       }
        );

        return instance;
    }

private
:
    scene_root() = default; // 禁用构造
};


static entt::entity &get_scene_root() {
    return scene_root::get();
}

inline std::vector<entt::entity> UI_stack_intersect(const Point_2 &current_position) {
    std::vector<entt::entity> return_value;
    auto scene_root_node = get_scene_root();
    return_value.push_back(scene_root_node);
    Scene_Component::check_entity_children_intersect_point(&return_value, scene_root_node, current_position);
    return return_value;
}


/**
 * 将一个节点添加到根节点
 * @param entity 必须存在Scene_Component，如果没有，会在这个函数中添加
 */
inline void scene_root_add_child(entt::entity entity) {
    auto root = get_scene_root();
    auto &parent_scene = get_entt_instance().get<Scene_Component>(root);
    auto &children_scene = get_entt_instance().get<Scene_Component>(entity);
    parent_scene.add_child(entity);
    children_scene.add_parent(entity);
}


#endif //HELLO_MAC_SCENE_COMPONENT_H

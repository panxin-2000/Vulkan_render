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
    entt::entity parent = entt::null;
    Point_2 zoom = {1, 1};
    Point_2 offset = {0, 0};
    std::vector<entt::entity> children;

public:
    void set_bounding_box(Point_2 min, Point_2 max) {
        bounding_box_ = AABB_centroid<Point_2>(min, max);
    }



    AABB_centroid<Point_2> bounding_box_; // 每次都直接计算吧。

    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = g_entt().try_get<Scene_Component>(entity)) {
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

    void add_parent(entt::entity entity) {
        parent = entity;
    }

    entt::entity get_parent() const {
        return parent;
    }


    void remove_parent() {
        parent = entt::null;
    }

    void remove_children(entt::entity entity) {
        children.erase(std::remove(children.begin(), children.end(), entity), children.end());
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
        const auto &storage = g_entt().storage<Scene_Component>();
        const auto entity = entt::to_entity(storage, *this);
        if (auto render = g_entt().try_get<logic_render_data *>(entity)) {
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
            (*render)->add_uniform("model_transform", gl_mat4, data);
        }
        return true;
    }

    bool update_position() {
        const auto &storage = g_entt().storage<Scene_Component>();


        const auto entity = entt::to_entity(storage, *this);
        if (auto render = g_entt().try_get<logic_render_data *>(entity)) {
            data_value_or_ptr data{};
            Shader_object::set_model_transform_zoom_rotate(data.vec_4,
                                                           {zoom.x, zoom.y, 1.0},
                                                           {0.0f, 0.0f, 0.0f}, {offset});
            (*render)->add_uniform("model_transform", gl_mat4, data);
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

// 回调函数
static inline void cleanup_logic_render_data(entt::registry &reg, const entt::entity ent) {
    if (const auto render_data = reg.try_get<logic_render_data *>(ent))
        clean_object_to_render(*render_data);
}

class scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = g_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           g_entt().emplace<Scene_Component>(instance);
                           g_entt().emplace<Name_component>(instance, "scene_root");
                           if (auto *scene_node = g_entt().try_get<Scene_Component>(instance)) {
                               scene_node->set_bounding_box({0, 0},
                                                            {
                                                                static_cast<float>(get_win_WIDTH()),
                                                                static_cast<float>(get_win_HEIGHT())
                                                            });
                           }
                           // 在系统初始化时
                           g_entt().on_destroy<logic_render_data *>().connect<&cleanup_logic_render_data>();
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
    const auto scene_root_node = get_scene_root();
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
    auto &parent_scene = g_entt().get<Scene_Component>(root);
    auto &children_scene = g_entt().get<Scene_Component>(entity);
    parent_scene.add_child(entity);
    children_scene.add_parent(root);
}

inline void scene_add_child(entt::entity parent_entity, entt::entity children_entity) {
    if (g_entt().all_of<Scene_Component>(parent_entity) &&
        g_entt().all_of<Scene_Component>(children_entity)) {
        auto &parent_scene = g_entt().get<Scene_Component>(parent_entity);
        auto &children_scene = g_entt().get<Scene_Component>(children_entity);

        parent_scene.add_child(children_entity);
        children_scene.add_parent(parent_entity);
    }
}

inline bool add_relation(const entt::entity parent_entity, const entt::entity children_entity) {
    if (g_entt().all_of<Scene_Component>(parent_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(parent_entity);
        entity_scene.add_child(children_entity);
    }
    if (g_entt().all_of<Scene_Component>(children_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(children_entity);
        entity_scene.add_parent(parent_entity);
    }
    return true;
}

static entt::entity get_parent(const entt::entity entity) {
    if (g_entt().all_of<Scene_Component>(entity)) {
        const auto &entity_scene = g_entt().get<Scene_Component>(entity);
        return entity_scene.get_parent();
    }
    return entt::null;
}

inline bool clear_relation(const entt::entity parent_entity, const entt::entity children_entity) {
    if (g_entt().all_of<Scene_Component>(parent_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(parent_entity);
        entity_scene.remove_children(children_entity);
    }
    if (g_entt().all_of<Scene_Component>(children_entity)) {
        auto &entity_scene = g_entt().get<Scene_Component>(children_entity);
        entity_scene.remove_parent();
    }
    return true;
}

inline bool clear_parent_relation(const entt::entity children_entity) {
    const auto parent_scene = get_parent(children_entity);
    clear_relation(parent_scene, children_entity);
    return true;
}

#endif //HELLO_MAC_SCENE_COMPONENT_H

//
// Created by 潘鑫 on 2026/5/22.
//
#include "Rect_2D_component.h"

#include <SDL3/SDL.h>

#include "bounding_box_2d.h"

bool check_entity_intersect_point(const entt::entity entity, const Eigen::Vector2f &current_position) {
    if (auto *scene_node = Logic_entt().try_get<Bounding_Box_2D>(entity)) {
        AABB_min_max<Point_2> temp{{scene_node->box[0], scene_node->box[1]}, {scene_node->box[2], scene_node->box[3]}};
        if (is_intersect(temp, {current_position.x(), current_position.y()})) {
            return true;
        }
    }
    return false;
}

bool get_intersect_entity(std::vector<entt::entity> &return_value,
                          const entt::entity entity,
                          const Eigen::Vector2f &mouse_position) {
    if (const auto *scene_node = Logic_entt().try_get<Scene_Component>(entity)) {
        for (const entt::entity children_entity: scene_node->children_) {
            if (check_entity_intersect_point(children_entity, mouse_position)) {
                return_value.push_back(children_entity);
                get_intersect_entity(return_value, children_entity, mouse_position);
            }
        }
    }
    return false;
}


bool deal_zoom(const entt::entity entity, const SDL_Event &event) {
    auto &transform = Logic_entt().get<Rect_2D_transform>(entity);
    transform.multiply_zoom(Point_2(
                                    static_cast<double>(std::pow(1.5f, event.wheel.x * 0.01f)),
                                    static_cast<double>(std::pow(1.5f, event.wheel.y * 0.01f))
                                   ));
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

bool deal_position_offset(const entt::entity entity, const SDL_Event &event) {
    auto &transform = Logic_entt().get<Rect_2D_transform>(entity);
    const Point_2 move{event.motion.xrel, event.motion.yrel};
    transform.add_offset(move);
    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
    return true;
}

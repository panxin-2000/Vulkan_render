//
// Created by 潘鑫 on 2026/5/22.
//
#include "Rect_2D_component.h"

bool check_entity_intersect_point(const entt::entity entity, const Point_2 &current_position) {
    if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
        if (intersect(scene_node->get_bounding_box(), current_position)) {
            return true;
        }
    }
    return false;
}

bool get_intersect_entity(std::vector<entt::entity> &return_value,
                          const entt::entity entity,
                          const Point_2 &mouse_position) {
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


bool deal_zoom(const entt::entity entity, const base_event_with_stamp &base_event) {
    auto &transform = Logic_entt().get<Rect_2D_transform>(entity);
    transform.multiply_zoom({
                                std::powf(1.5, base_event.scroll.x * 0.01),
                                std::powf(1.5, base_event.scroll.y * 0.01)
                            });
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

bool deal_position_offset(const entt::entity entity, const base_event_with_stamp &base_event) {
    auto &transform     = Logic_entt().get<Rect_2D_transform>(entity);
    const Point_2 move = base_event.current_position - base_event.last_position;
    transform.add_offset(move);
    Logic_entt().emplace_or_replace<UI_transform_dirty>(entity);
    return true;
}

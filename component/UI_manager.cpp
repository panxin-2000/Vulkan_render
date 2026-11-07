//
// Created by 潘鑫 on 2026/5/22.
//

#include "UI_manager.h"


std::vector<entt::entity> UI_stack_intersect(const Eigen::Vector2f &current_position) {
    std::vector<entt::entity> return_value;
    const auto scene_root_node = get_UI_scene_root();
    get_intersect_entity(return_value, scene_root_node, current_position);
    return return_value;
}

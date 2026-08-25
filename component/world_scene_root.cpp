//
// Created by 潘鑫 on 2026/8/21.
//

#include "world_scene_root.h"

#include "camera_optical_component.h"
#include "input_component.h"
#include "move_speed.h"
#include "name_component.h"
#include "sync_proxy_to_render_thread.h"


void update_camera_parameter(const entt::entity entity);


wmOperatorStatus view_move_up(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count() / 1000.f;
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_direction() * -pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}

wmOperatorStatus view_move_down(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count() / 1000.f;
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_direction() * pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}

wmOperatorStatus view_move_left(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count() / 1000.f;
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_right_direction() * -pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}

wmOperatorStatus view_move_right(const entt::entity entity, std::chrono::milliseconds ms) {
    float speed = 1.0f;
    if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
        speed = move_speed->speed;
    }
    float pos_err = speed * ms.count() / 1000.f;
    if (const auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto offset = camera->get_view_right_direction() * pos_err;
        camera->add_offset(offset);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
    }
    return OPERATOR_FINISHED;
}


static wmOperatorStatus world_rotate(const entt::entity entity, const Point_2 temp) {
    if (auto camera = Logic_entt().try_get<camera_optical_component>(entity)) {
        auto q_current = camera->get_rotate();
        q_current      = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY()) *
                                       q_current);
        q_current =
                q_current * Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX()));
        q_current.normalize();
        camera->set_rotate(q_current);
        Logic_entt().emplace_or_replace<Camera_dirty>(entity);
        return OPERATOR_FINISHED;
    } else {
        return OPERATOR_PASS_THROUGH;
    }
}


void init_world_scene_root(entt::entity entity) {
    Logic_entt().emplace<Scene_Component>(entity);
    Logic_entt().emplace<Name_component>(entity, "world_scene_root");
    Logic_entt().emplace<Move_speed>(entity);

    update_camera_parameter(entity);

    auto &input = Logic_entt().emplace<Input_Component>(entity);
    input.add_shortcut_keys(Combined_shortcut_keys(std::vector<std::string>{"w"}), view_move_up, true);
    input.add_shortcut_keys(Combined_shortcut_keys(std::vector<std::string>{"s"}), view_move_down, true);
    input.add_shortcut_keys(Combined_shortcut_keys(std::vector<std::string>{"a"}), view_move_left, true);
    input.add_shortcut_keys(Combined_shortcut_keys(std::vector<std::string>{"d"}), view_move_right, true);

    input.add_scroll(world_rotate);
}

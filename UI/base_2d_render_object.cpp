//
// Created by 潘鑫 on 2026/8/22.
//

#include "base_2d_render_object.h"

#include "bounding_box_2d.h"
#include "input_component.h"
#include "name_component.h"
#include "scene_component.h"
#include "UI_button.h"


static wmOperatorStatus model_offset(const entt::entity entity,
                                     const Eigen::Vector2f current_position,
                                     const Eigen::Vector2f last_position) {
    if (auto *transform = Logic_entt().try_get<Bounding_Box_2D>(entity)) {
        Eigen::Vector4f temp;

        temp.head<2>()         = current_position - last_position; // 将 v1 赋给 v4 的前两个元素
        temp.tail<2>()         = current_position - last_position; // 将 v2 赋给 v4 的后两个元素
        Eigen::Vector4f result = transform->box + temp;
        set_round_box_impl(entity, result);

        return OPERATOR_RUNNING_MODAL;
    } else {
        return OPERATOR_PASS_THROUGH;
    }
}


void add_model_2d_Event(const entt::entity entity) {
    auto &status = Logic_entt().emplace_or_replace<Input_Component>(entity);
    status.add_mouse_drag(model_offset);
    status.add_shortcut_keys(Combined_shortcut_keys(" 'x' "), delete_object);
}


object_2d::object_2d(const std::string &name) : logic_render_object(name) {
    logic_create_proxy(entity);
    add_model_2d_Event(entity); // 这里不对,需要改为2d的
    logic_update_proxy<Name_component>(entity);
    UI_root_add_child(entity);
}

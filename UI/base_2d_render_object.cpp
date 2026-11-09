//
// Created by 潘鑫 on 2026/8/22.
//

#include "UI/base_2d_render_object.h"

#include "bounding_box_2d.h"
#include "input_component.h"
#include "name_component.h"
#include "scene_component.h"
#include "UI/UI_button.h"


void set_render_2D_data_dirty(const entt::entity itself_entity);

static wmOperatorStatus model_offset(const entt::entity entity,
                                     const Eigen::Vector2f current_position,
                                     const Eigen::Vector2f last_position) {
    if (auto *data = Logic_entt().try_get<UI_Button_data>(entity)) {
        // 这一步做的内容是更改值
        Eigen::Vector4f temp;
        temp.head<2>() = current_position - last_position; // 将 v1 赋给 v4 的前两个元素
        temp.tail<2>() = current_position - last_position; // 将 v2 赋给 v4 的后两个元素
        data->box      = data->box + temp;
        if (auto *bounding_box = Logic_entt().try_get<Bounding_Box_2D>(entity)) {
            bounding_box->box += temp;
        }

        // 这里做的内容是更改标记
        set_render_2D_data_dirty(entity);

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


object_2d::object_2d(const std::string &name, const entt::entity parent_entity) : logic_render_object(name) {
    add_model_2d_Event(entity);
    if (parent_entity != entt::null) {
        add_relation(parent_entity, entity);
    } else {
        UI_root_add_child(entity);
    }
}

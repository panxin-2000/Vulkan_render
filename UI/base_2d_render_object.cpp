//
// Created by 潘鑫 on 2026/8/22.
//

#include "UI/base_2d_render_object.h"

#include "bounding_box_2d.h"
#include "input_component.h"
#include "name_component.h"
#include "scene_component.h"
#include "UI/UI_button.h"


#include  "UI_render_common.h"


float sdRoundedBox(Eigen::Vector2f p, Eigen::Vector2f b, Eigen::Vector4f r) {
    // 根据 p.x 和 p.y 的象限，直接映射到对应的圆角半径分量
    const float rad = (p.x() > 0.0f)
                          ? ((p.y() > 0.0f) ? r.x() : r.y())
                          : ((p.y() > 0.0f) ? r.z() : r.w());
    const Eigen::Vector2f q          = p.cwiseAbs() - b + Eigen::Vector2f::Constant(rad);
    const float max_q_xy             = std::max(q.x(), q.y());
    const Eigen::Vector2f max_q_zero = q.cwiseMax(0.0f);
    return std::min(max_q_xy, 0.0f) + max_q_zero.norm() - rad;
}


static wmOperatorStatus model_offset(const entt::entity entity,
                                     const Eigen::Vector2f &current_position,
                                     const Eigen::Vector2f &last_position) {
    if (Logic_entt().any_of<UI_Button_data, Bounding_Box_2D>(entity)) {
        // 这一步做的内容是更改值
        Eigen::Vector4f temp;
        temp.head<2>() = current_position - last_position; // 将 v1 赋给 v4 的前两个元素
        temp.tail<2>() = current_position - last_position; // 将 v2 赋给 v4 的后两个元素

        if (auto *data = Logic_entt().try_get<UI_Button_data>(entity)) {
            float min_x              = data->box.x();
            float min_y              = data->box.y();
            float max_x              = data->box.z();
            float max_y              = data->box.w();
            Eigen::Vector2f center   = Eigen::Vector2f((min_x + max_x) / 2, (min_y + max_y) / 2);
            Eigen::Vector2f half_box = Eigen::Vector2f((max_x - min_x) / 2, (max_y - min_y) / 2);
            float sd                 = sdRoundedBox(current_position - center, half_box, Eigen::Vector4f::Zero());
            if (std::abs(sd) < 10) {
                if (center.x() > current_position.x()) {
                    data->box.x() += (current_position - last_position).x();
                } else {
                    data->box.z() += (current_position - last_position).x();
                }
                if (center.y() > current_position.y()) {
                    data->box.y() += (current_position - last_position).y();
                } else {
                    data->box.w() += (current_position - last_position).y();
                }
            } else {
                data->box = data->box + temp;
            }
            Logic_entt().emplace_or_replace<Bounding_Box_2D>(entity, data->box);
            set_render_2D_data_dirty(entity);
            return OPERATOR_RUNNING_MODAL;
        }
        if (auto *bounding_box = Logic_entt().try_get<Bounding_Box_2D>(entity)) {
            bounding_box->box += temp;
        }
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

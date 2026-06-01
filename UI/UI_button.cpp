//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "model_matrix.h"
#include "name_component.h"
#include "Rect_2D_component.h"

static wmOperatorStatus on_Event(const entt::entity entity, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;
    auto &status   = Logic_entt().get<Input_Component>(entity);

    switch (temp_type) {
        case EVT_KEY_X:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    Logic_entt().emplace_or_replace<Logic_destroy_tag>(entity);
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        case EVT_KEY_ESCAPE:
            if (event.event_code == KM_PRESS) {
                // std::cout << " button  EVT_KEY_ESCAPE KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态 先用按下的状态，之后再更改
                return OPERATOR_CANCELLED;
            }
            break;
        case MOUSE_LEFT:
            if (event.event_code == KM_PRESS) {
                status.select_status_ = select_current;
                // std::cout << " button  MOUSE_LEFT KM_PRESS" << std::endl;
                // 需要增加模态的处理 返回锁定模态
                return OPERATOR_RUNNING_MODAL;
            }
            if (event.event_code == KM_RELEASE) {
                // std::cout << " button  MOUSE_LEFT KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态
                // auto block_entity = UI_button("新按钮", 10, 10, 220, 220);
                status.select_status_ = no_select_current;
                return OPERATOR_FINISHED;
            }
            break;
        case MOUSE_RIGHT:
            break;
        case WHEEL_UP_MOUSE:
            if (auto *UI = Logic_entt().try_get<Rect_2D_transform>(entity)) {
                deal_zoom(entity, event);
            }
            break;
        case MOUSE_MOVE:
            if (status.select_status_ == select_current) {
                if (auto *UI = Logic_entt().try_get<Rect_2D_transform>(entity)) {
                    deal_position_offset(entity, event);
                    // 包围盒的位置还需要同步更新
                    return OPERATOR_RUNNING_MODAL;
                }
            }
            break;
        default:
            return OPERATOR_PASS_THROUGH;
    }
    return OPERATOR_HANDLED;
}


entt::entity UI_button(const std::string &name,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y) {
    // std::cout << "UI_button" << std::endl;
    std::string_view df = "";

    LOG_INFO(g_log(), "UI create  {} {} {} {} {} ", name, min_x, min_y, max_x, max_y);

    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);

    /***************创建*******************/
    Logic_entt().emplace<Input_Component>(entity, on_Event);

    Logic_entt().emplace<Rect_2D_transform>(entity);
    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_round_box.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_round_box.frag.spv",
               "", ""); // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP 暂时还不用

    if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
    Logic_entt().emplace<Drag_event>(entity);
    Logic_entt().emplace<Name_component>(entity, name);
    add_2D_bound_box_geometry(entity, {min_x, min_y}, {max_x, max_y});


    matrix_4x4 model;
    UI_matrix_4x4(&model, {1, 1}, {0, 0});
    set_render_parameter(entity, "model_4x4", model);
    struct Round_box {
        float min_x, min_y, max_x, max_y;
        float radius_min_x, radius_min_y, radius_max_x, radius_max_y;
    };
    Round_box round_box = {min_x, min_y, max_x, max_y, 20, 10, 10, 10};
    set_render_parameter(entity, "round_box", round_box);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    logic_update_add_tag<UI_2D_tag>(entity);

    float scale[2];
    scale[0] = 2.0f / 1280.f;
    scale[1] = 2.0f / 720;
    float translate[2];
    translate[0] = -1.0f - 0.0f * scale[0];
    translate[1] = -1.0f - 0.0f * scale[1];

    set_push_constant_parameter(entity, "uScale", scale);
    set_push_constant_parameter(entity, "uTranslate", translate);
    scene_root_add_child(entity);
    return entity;

    // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
    /***************添加到渲染管理器**********************/
}

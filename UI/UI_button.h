//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_UI_BUTTON_H
#define HELLO_MAC_UI_BUTTON_H


#include "input_component.h"
#include "base_event.h"
#include "name_component.h"
#include "global_singleton.h"
#include "observer_manage.h"
#include "span.hpp"
#include "Rect_2D_component.h"

entt::entity UI_button(const std::string &name,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y);

// 按键和鼠标有两种截然不同的策略，基本上，所以的鼠标的点击时并没有反应，但是呢？
// 松开时 采取执行按键设计的动作

// 键盘是另一种操作，按下时就去执行响应的动作，有时会增加弹窗来进行确认

static wmOperatorStatus on_Event(const entt::entity entity_, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;
    auto &status   = g_entt().get<Input_Component>(entity_);

    switch (temp_type) {
        case EVT_KEY_X:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (g_entt().valid(entity_)) {
                    // if (const auto render = g_entt().try_get<logic_render_data>(entity_)) {
                    //     render->proxy = nullptr;
                    // }
                    // 加上上面的内容就有问题
                    g_entt().emplace_or_replace<Destroy_tag>(entity_);
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        case EVT_KEY_ESCAPE:
            if (event.event_code == KM_PRESS) {
                std::cout << " button  EVT_KEY_ESCAPE KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态 先用按下的状态，之后再更改
                return OPERATOR_CANCELLED;
            }
            break;
        case MOUSE_LEFT:
            if (event.event_code == KM_PRESS) {
                status.select_status = select_current;
                std::cout << " button  MOUSE_LEFT KM_PRESS" << std::endl;
                // 需要增加模态的处理 返回锁定模态
                return OPERATOR_RUNNING_MODAL;
            }
            if (event.event_code == KM_RELEASE) {
                std::cout << " button  MOUSE_LEFT KM_RELEASE" << std::endl;
                // 需要增加模态的处理 返回结束模态
                // auto block_entity = UI_button("新按钮", 10, 10, 220, 220);
                status.select_status = no_select_current;
                return OPERATOR_FINISHED;
            }
            break;
        case MOUSE_RIGHT:
            break;
        case WHEEL_UP_MOUSE:
            if (auto *UI = g_entt().try_get<Rect_2D_transform>(entity_)) {
                UI->set_zoom(entity_, event);
            }
            break;
        case MOUSE_MOVE:
            if (status.select_status == select_current) {
                if (auto *UI = g_entt().try_get<Rect_2D_transform>(entity_)) {
                    UI->set_position_offset(entity_, event);
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
    std::cout << "UI_button" << std::endl;
    std::string_view df = "";

    LOG_INFO(g_log(), "UI create  {} {} {} {} {} ", name, min_x, min_y, max_x, max_y);

    entt::entity entity_ = g_entt().create();

    /***************创建*******************/
    g_entt().emplace<Input_Component>(entity_, on_Event);

    g_entt().emplace<Scene_Component>(entity_);
    g_entt().emplace<Rect_2D_transform>(entity_);
    g_entt().emplace<VKR_shader_paths>(entity_,
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                       "", "");

    if (auto *scene_node = g_entt().try_get<Rect_2D_transform>(entity_)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
    g_entt().emplace<Drag_event>(entity_);
    g_entt().emplace<Name_component>(entity_, name);
    add_geometry_data(entity_, min_x, min_y, max_x, max_y);


    matrix_4x4 model;
    UI_matrix_4x4(&model, 1280, 720);
    set_render_parameter(entity_, "model_4x4", model);

    g_entt().emplace_or_replace<add_to_render_tag>(entity_);

    if (g_entt().all_of<Scene_Component>(entity_)) {
        scene_root_add_child(entity_);
    }
    return entity_;

    // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
    /***************添加到渲染管理器**********************/
}

#endif //HELLO_MAC_UI_BUTTON_H

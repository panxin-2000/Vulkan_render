//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "model_matrix.h"
#include "name_component.h"
#include "Rect_2D_component.h"


struct Round_box {
    float min_x, min_y, max_x, max_y;
    float radius_min_x, radius_min_y, radius_max_x, radius_max_y;
};


static wmOperatorStatus on_Event(const entt::entity entity, const SDL_Event &event) {
    auto &status = Logic_entt().get<Input_Component>(entity);
    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            if (status.select_status_ == select_current) {
                auto &round_box = Logic_entt().get<Round_box>(entity);
                round_box.min_x += event.motion.xrel;
                round_box.min_y += event.motion.yrel;
                round_box.max_x += event.motion.xrel;
                round_box.max_y += event.motion.yrel;
                set_render_parameter(entity, "round_box", round_box);
                return OPERATOR_RUNNING_MODAL;
            }
            return OPERATOR_PASS_THROUGH;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            status.select_status_ = select_current;
            return OPERATOR_RUNNING_MODAL;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            status.select_status_ = no_select_current;
            return OPERATOR_FINISHED;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            // event.wheel.x
            // if (auto *UI = Logic_entt().try_get<Rect_2D_transform>(entity)) {
            //     deal_zoom(entity, event);
            // }
        }
        case SDL_EVENT_TEXT_INPUT: {
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
        }
        case SDL_EVENT_FINGER_MOTION: {
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER: {
        }
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
        }
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED: {
        }
        default:
            break;
    }
    return OPERATOR_PASS_THROUGH;
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
    // Logic_entt().emplace<Input_Component>(entity, on_Event);

    Logic_entt().emplace<Rect_2D_transform>(entity);
    add_shader(entity,
               "2D/vulkan_round_box",
               "2D/vulkan_round_box",
               "", ""); // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP 暂时还不用

    if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
    Logic_entt().emplace<Name_component>(entity, name);
    add_2D_bound_box_geometry(entity, {min_x, min_y}, {max_x, max_y});

    matrix_4x4 model;
    UI_matrix_4x4(&model, {1, 1}, {0, 0});
    set_render_parameter(entity, "model_4x4", model);

    Round_box &round_box = Logic_entt().emplace<
        Round_box>(entity, Round_box{min_x, min_y, max_x, max_y, 20, 10, 10, 10});

    set_render_parameter(entity, "round_box", round_box);
    // 然后应该想办法做什么呢？
    // 要么是想办法更新顶点 ，要么是

    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    for (auto &primitive: primitives) {
        // primitive.pipelineDynamicState.depthTestEnable  = VK_FALSE;  // TODP :暂时不用,先不做修改
        // primitive.pipelineDynamicState.depthWriteEnable = VK_FALSE;
        // primitive.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
        // primitive.set_VkCullModeFlags(VK_CULL_MODE_NONE);
        // 也就是 UI 部分的 代码，其实 有时候是没有办法去管理三角形的 大小的
    }
    logic_update_proxy(entity, primitives);
    logic_update_add_tag<UI_2D_tag>(entity);

    float scale[2];
    scale[0] = 2.0f / 1280.f;
    scale[1] = 2.0f / 800;
    float translate[2];
    translate[0] = -1.0f - 0.0f * scale[0];
    translate[1] = -1.0f - 0.0f * scale[1];

    set_push_constant_parameter(entity, "uScale", scale);
    set_push_constant_parameter(entity, "uTranslate", translate);
    UI_root_add_child(entity);
    return entity;

    // 还想需要添加位置的，以及缩放。缩放暂时不需要，需要添加层。
    /***************添加到渲染管理器**********************/
}

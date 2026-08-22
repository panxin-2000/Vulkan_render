//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "model_matrix.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "render_state.h"


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


UI_Button &UI_Button::set_round_box(float min_x,
                                    float min_y,
                                    float max_x,
                                    float max_y) {
    LOG_INFO(g_log(), "UI create  {} {} {} {} ", min_x, min_y, max_x, max_y);
    add_2D_bound_box_geometry(entity, {min_x, min_y}, {max_x, max_y});
    Round_box &round_box = Logic_entt().emplace<
        Round_box>(entity, Round_box{min_x, min_y, max_x, max_y, 20, 10, 10, 10});
    set_render_parameter(entity, "round_box", round_box);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    std::vector<VKR_Render_state> render_states;
    VKR_Render_state render_state;
    render_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    render_state.set_VkCullModeFlags(VK_CULL_MODE_NONE);
    render_states.push_back(render_state);
    logic_update_proxy(entity, render_states);

    return *this;
}

UI_Button::UI_Button(const std::string &name) : object_2d(name) {
    add_shader_path(VKR_shader_paths{
                        "2D/vulkan_round_box", "2D/vulkan_round_box", "", ""
                    });
    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();

    logic_update_add_tag<UI_2D_tag>(entity);

    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);

    float scale[2];
    scale[0] = 2.0f / logical_w; // Scale
    scale[1] = 2.0f / logical_h;
    float translate[2];
    translate[0] = -1.0f - 0 * scale[0]; // Translate
    translate[1] = -1.0f - 0 * scale[1];

    add_push_constant_parameter("uScale", scale);
    add_push_constant_parameter("uTranslate", translate);
}

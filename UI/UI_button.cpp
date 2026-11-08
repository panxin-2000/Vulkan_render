//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI/UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "render_component/Geometry_component.h"
#include "model_matrix.h"
#include "../include/name_component.h"
#include "Rect_2D_component.h"
#include "render_state.h"
#include "UI/UI_text.h"


#include  "bounding_box_2d.h"

//


static wmOperatorStatus on_Event(const entt::entity entity, const SDL_Event &event) {
    auto &status = Logic_entt().get<Input_Component>(entity);
    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            if (status.select_status_ == select_current) {
                auto &round_box   = Logic_entt().get<Bounding_Box_2D>(entity);
                round_box.min_x() += event.motion.xrel;
                round_box.min_y() += event.motion.yrel;
                round_box.max_x() += event.motion.xrel;
                round_box.max_y() += event.motion.yrel;
                logic_set_render_parameter(entity, "round_box", round_box);
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


void set_round_box_impl(const entt::entity entity,
                        const float min_x,
                        const float min_y,
                        const float max_x,
                        const float max_y) {
    Logic_entt().emplace_or_replace<Bounding_Box_2D>(entity, Eigen::Vector4f(min_x, min_y, max_x, max_y));
    LOG_INFO(g_log(), "UI create  {} {} {} {} ", min_x, min_y, max_x, max_y);
    add_2D_bound_box_geometry(entity, {min_x, min_y}, {max_x, max_y});

    auto RoundRect = Logic_entt().get<UI_Button_data>(entity);
    logic_set_render_parameter(entity, "round_box", RoundRect);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    auto primitives = create_primitives(entity);
    logic_update_proxy(entity, primitives);
    std::vector<VKR_Render_state> render_states;
    VKR_Render_state render_state;
    render_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    render_state.set_VkCullModeFlags(VK_CULL_MODE_NONE);
    render_state.set_depthTest_disable();
    render_states.push_back(render_state);
    logic_update_proxy(entity, render_states);
}


void set_round_box_impl(const entt::entity entity,
                        const Eigen::Vector4f &box) {
    set_round_box_impl(entity, box[0], box[1], box[2], box[3]);
}


UI_Button &UI_Button::set_Round_Rect(Eigen::Vector4f box,
                                     float right_down,
                                     float right_up,
                                     float left_down,
                                     float left_up) {
    auto data = Logic_entt().emplace_or_replace<UI_Button_data>(entity,
                                                                UI_Button_data{
                                                                    box, right_down, right_up, left_down, left_up
                                                                });

    Logic_entt().emplace_or_replace<Bounding_Box_2D>(entity, box);

    set_round_box_impl(entity, box);
    return *this;
}

UI_Button &UI_Button::set_Round_Rect(float min_x,
                                     float min_y,
                                     float max_x,
                                     float max_y,
                                     float right_down,
                                     float right_up,
                                     float left_down,
                                     float left_up) {
    auto data = Logic_entt().emplace_or_replace<UI_Button_data>(entity, UI_Button_data{
                                                                    {min_x, min_y, max_x, max_y},
                                                                    right_down, right_up, left_down, left_up
                                                                });
    Logic_entt().emplace_or_replace<Bounding_Box_2D>(entity, Eigen::Vector4f{min_x, min_y, max_x, max_y});


    set_round_box_impl(entity, min_x, min_y, max_x, max_y);
    return *this;
}

UI_Button::UI_Button(const std::string &debug_name)
    : object_2d(debug_name) {
    add_shader_path(VKR_shader_paths{
                        "2D/vulkan_round_box", "2D/vulkan_round_box", "", ""
                    });
    int logical_w, logical_h;
    const auto &backend = VK_backend::instance();

    logic_update_add_tag<UI_2D_tag>(entity);

    SDL_GetWindowSize(backend.get_window(), &logical_w, &logical_h);

    Eigen::Vector2f scale{2.0f / logical_w, 2.0f / logical_h};
    Eigen::Vector2f translate{-1.0f - 0 * scale[0], -1.0f - 0 * scale[1]};

    add_push_constant_parameter("uScale", scale);
    add_push_constant_parameter("uTranslate", translate);
}

void UI_Button::draw() const {
}

void UI_Button::update() const {
    Logic_entt().emplace_or_replace<Tag_UI_Button_data_dirty>(entity);
}

void UI_Button::set_renderable(const bool renderable) const {
    if (renderable == true) {
        Logic_entt().emplace_or_replace<Tag_UI_Button_renderable>(entity);
    } else {
        Logic_entt().remove<Tag_UI_Button_renderable>(entity);
    }
}


UI_Text UI_Button::add_text(const std::string &debug_name) const {
    const UI_Text text(debug_name);
    add_relation(entity, text.get_entity());
    return text;
}

UI_Button UI_Button::add_button(const std::string &debug_name) const {
    const UI_Button button("debug_name");
    add_relation(entity, button.get_entity());
    return button;
}

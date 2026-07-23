//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "model_matrix.h"
#include "name_component.h"
#include "Rect_2D_component.h"

static wmOperatorStatus on_Event(const entt::entity entity, const SDL_Event &event) {
    auto &status = Logic_entt().get<Input_Component>(entity);
    switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            if (status.select_status_ == select_current)
                if (auto *UI = Logic_entt().try_get<Rect_2D_transform>(entity)) {
                    deal_position_offset(entity, event);
                    // 包围盒的位置还需要同步更新
                    return OPERATOR_RUNNING_MODAL;
                }
            return OPERATOR_PASS_THROUGH;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            if (auto *UI = Logic_entt().try_get<Rect_2D_transform>(entity)) {
                deal_zoom(entity, event);
            }
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            status.select_status_ = select_current;
            return OPERATOR_RUNNING_MODAL;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            status.select_status_ = no_select_current;
            return OPERATOR_FINISHED;
        }
        case SDL_EVENT_TEXT_INPUT: {
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            if (event.key.key == SDLK_W && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({0, 0, -1});
                    Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_S && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({0, 0, -1});
                    Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_A && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({1, 0, 0});
                    Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_D && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    position->add_offset({1, 0, 0});
                    Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_X && Logic_entt().valid(entity)) {
                if (Logic_entt().valid(entity)) {
                    Logic_entt().emplace_or_replace<Logic_destroy_tag>(entity);
                    return OPERATOR_FINISHED;
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_SPACE && Logic_entt().valid(entity)) {
                if (auto position = Logic_entt().try_get<Transform>(entity)) {
                    if (event.key.mod & SDL_KMOD_SHIFT)
                        position->add_offset({0, -1, 0});
                    else
                        position->add_offset({0, 1, 0});
                    Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                }
                return OPERATOR_FINISHED;
            } else if (event.key.key == SDLK_ESCAPE && Logic_entt().valid(entity)) {
                return OPERATOR_CANCELLED;
            } else {
                return OPERATOR_PASS_THROUGH;
            }
        }
        case SDL_EVENT_FINGER_MOTION: {
            const Point_2 temp{event.tfinger.dx, event.tfinger.dy};
            if (auto position = Logic_entt().try_get<Transform>(entity)) {
                auto q_current = position->get_rotate();
                q_current = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY()) * q_current);
                q_current = q_current * Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX()));
                position->set_rotate(q_current);
                Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                return OPERATOR_FINISHED;
            } else {
                return OPERATOR_PASS_THROUGH;
            }
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
    Logic_entt().emplace<Input_Component>(entity, on_Event);

    Logic_entt().emplace<Rect_2D_transform>(entity);
    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_round_box.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_round_box.frag.spv",
               "", ""); // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP 暂时还不用

    if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(entity)) {
        scene_node->set_bounding_box({min_x, min_y}, {max_x, max_y});
    }
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

    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
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

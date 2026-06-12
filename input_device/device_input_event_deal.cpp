#include <GLFW/glfw3.h>

#include "transform_component.h"
#include "../event/base_event.h"
#include "name_component.h"
#include "global_singleton.h"
#include "input_component.h"
#include "../event/input_device_manage.h"
#include "scene_component.h"
#include "Rect_2D_component.h"
#include "UI_manager.h"
#include "base_geometry/base.h"


void glfwFocusCallback(GLFWwindow *window, int focused);

void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

entt::dispatcher dispatcher;

void base_event_dealing(const base_event_with_stamp &event);


void deal_glfw_event() {
    dispatcher.update();
}


void register_glfw(GLFWwindow *window) {
    dispatcher.sink<base_event_with_stamp>().connect<&base_event_dealing>();


    Keyboard_Manage::instance().init_eventQueueMgr(&dispatcher);
    glfwSetWindowFocusCallback(window, glfwFocusCallback); // 窗口焦点回调
    glfwSetKeyCallback(window, glfwKeyCallback);           // 键盘事件回调

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // glfwSetCursorEnterCallback(window, function_name);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
}


// GLFW窗口焦点回调（失去焦点时清空按键状态）
void glfwFocusCallback(GLFWwindow *window, int focused) {
    if (focused == GLFW_FALSE) {
        Keyboard_Manage::instance().clear_focus();
        glfwSetKeyCallback(window, nullptr); // 键盘事件回调

        // glfwSetInputMode(window, GLFW_CURSOR, nullptr);
        glfwSetCursorPosCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);
        // glfwSetCursorEnterCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
    } else {
        Keyboard_Manage::instance().set_focus();
        glfwSetKeyCallback(window, glfwKeyCallback); // 键盘事件回调

        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        // glfwSetCursorEnterCallback(window, function_name);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    LOG_INFO(g_log(), "scroll_callback x: {} y: {} ", x_offset, y_offset);

    Keyboard_Manage::instance().handle_scroll({static_cast<float>(x_offset), static_cast<float>(y_offset)});
}


// Window dimensions


/**
 * 鼠标点击之后才会调用这个函数
 * @param window
 * @param button
 * @param action
 * @param mods
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    double x_pos;
    double y_pos;

    glfwGetCursorPos(window, &x_pos, &y_pos);
    LOG_INFO(g_log(), "mouse x: {:4d} y: {:4d} ", (int)x_pos, (int)y_pos);
    // 由窗口具体的窗口自己去更改吧。
    // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
    // 更改坐标系的范围，x轴是从左到右，范围是-1到1之间，y轴是从下到上，范围是-1到1之间
#define key_instance Keyboard_Manage::instance()
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        key_instance.handle_mouse_click_left({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        key_instance.handle_mouse_release_left({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        key_instance.handle_mouse_click_right({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        key_instance.handle_mouse_release_right({static_cast<float>(x_pos), static_cast<float>(y_pos)});
    }
#undef key_instance
}


void glfwKeyCallback(GLFWwindow *window, const int key, int scancode, const int action, int mods) {
    // std::cout << "key: " << key << std::endl;

    if (key == GLFW_KEY_UNKNOWN) return;
    // action: GLFW_PRESS（按下）、GLFW_RELEASE（松开）、GLFW_REPEAT（重复按下）
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(window, GL_TRUE);
    //     return;
    // }
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        Keyboard_Manage::instance().handleKeyDown(key);
    } else if (action == GLFW_RELEASE) {
        Keyboard_Manage::instance().handleKeyUp(key);
    }
}


// 只要鼠标动了就会调用这里
void mouse_callback(GLFWwindow *window, const double x_pos, const double y_pos) {
    // x_pos = ((x_pos / get_win_WIDTH()) - 0.5f) * 2, y_pos = ((y_pos / get_win_HEIGHT()) - 0.5f) * -2;
    Keyboard_Manage::instance().handle_drag({static_cast<float>(x_pos), static_cast<float>(y_pos)});
}


static wmOperatorStatus world_root_on_Event(const entt::entity entity, const base_event_with_stamp &event) {
    auto temp_type = event.event_type;

    switch (temp_type) {
        case MOUSE_ROTATE: {
            auto temp = event.scroll;

            if (auto position = Logic_entt().try_get<Transform>(entity)) {
                auto q_current = position->get_rotate();
                q_current = Eigen::Quaternionf(Eigen::AngleAxisf(temp.x / 100, Eigen::Vector3f::UnitY()) * q_current);
                q_current = q_current * Eigen::Quaternionf(Eigen::AngleAxisf(temp.y / 100, Eigen::Vector3f::UnitX()));
                position->set_rotate(q_current);
                Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
            }
        }
        case EVT_KEY_W:
            // 删除当前鼠标位置的元素
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({0, 0, -1});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
        case EVT_KEY_S: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({0, 0, 1});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        case EVT_KEY_A: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({-1, 0, 0});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        case EVT_KEY_D: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        position->add_offset({1, 0, 0});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        case EVT_KEY_SPACE_KEY: {
            if (event.event_code == KM_PRESS)
                if (Logic_entt().valid(entity)) {
                    if (auto position = Logic_entt().try_get<Transform>(entity)) {
                        if (event.modifier_flag & KM_SHIFT)
                            position->add_offset({0, -1, 0});
                        else
                            position->add_offset({0, 1, 0});
                        Logic_entt().emplace_or_replace<Camera_transform_dirty>(entity);
                    }
                    return OPERATOR_FINISHED;
                }
            return OPERATOR_PASS_THROUGH;
            break;
        }
        default: {
        }
    }
}


#include "base_geometry/intersect_function.h"

entt::entity find_entity_insert_ray(Ray<Point_3> &ray) {
    const auto view = Logic_entt().view<Name_component, AABB_centroid<Point_3>, Transform>();
    for (auto &entity: view) {
        auto position = view.get<Transform>(entity);
        auto box      = view.get<AABB_centroid<Point_3> >(entity);
        box.add_offset(position.get_position());
        if (is_intersect(box, ray)) {
            auto &name = view.get<Name_component>(entity);
            LOG_INFO(g_log(), " insert box 3d {} ", name.name_);
            return entity;
        }
    }

    return entt::null;
}


void base_event_dealing(const base_event_with_stamp &event) {
    const auto view = Logic_entt().view<Name_component, Scene_Component, Input_Component>();


    static entt::entity current_select_entity = get_UI_scene_root();
    const mouse_position current_position     = event.current_position;
    static wmOperatorStatus current_status    = OPERATOR_ZERO;

    // 鼠标按下时进入模态，移动时，持续模态，鼠标松开时 完成模态 ，按下 ESC 键时，取消模态（ 取消后按键依旧按下，处理需谨慎）
    // 按下 ESC 键时，取消操作，模态已经在，之后的时间不处理，只等鼠标松开取消模态
    auto &name = view.get<Name_component>(current_select_entity);
    // std::cout << "last work name: " << name.name << std::endl;
    if (current_status == OPERATOR_RUNNING_MODAL)
        if (const auto input = Logic_entt().try_get<Input_Component>(current_select_entity)) {
            if (input->on_Event != nullptr) {
                auto status = input->on_Event(current_select_entity, event);
                if (OPERATOR_RUNNING_MODAL & status) {
                    current_select_entity = current_select_entity; // 目的是更新，但是没有什么意义
                    current_status        = OPERATOR_RUNNING_MODAL;
                    return;
                } else if (OPERATOR_FINISHED & status) {
                    current_status        = OPERATOR_ZERO;
                    current_select_entity = get_UI_scene_root();
                    return;
                }
                current_status = OPERATOR_ZERO;
            }
        }

    // 找到当前区域的一个递归栈

    auto ray = get_screen_ray(event.current_position);
    LOG_INFO(g_log(), "ray {}  {}  {}   direction {} {} {}  ", ray.point.x, ray.point.y, ray.point.z,
             ray.direction.x, ray.direction.y, ray.direction.z);


    std::vector<entt::entity> UI_stack = UI_stack_intersect(current_position);
    // std::cout << "UI stack size: " << UI_stack.size() << std::endl;
    // for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
    //     auto &name = view.get<Name_component>(*it);
    //     std::cout << "name: " << name.name << std::endl;
    // }

    if (const auto insert_entity = find_entity_insert_ray(ray); insert_entity != entt::null) {
        UI_stack.push_back(insert_entity);
    }
    for (auto it = UI_stack.rbegin(); it != UI_stack.rend(); ++it) {
        if (const auto input = Logic_entt().try_get<Input_Component>(*it)) {
            if (input->on_Event != nullptr) {
                auto status = input->on_Event(*it, event);
                if (OPERATOR_RUNNING_MODAL & status) {
                    current_select_entity = *it;
                    current_status        = OPERATOR_RUNNING_MODAL;
                    break;
                } else if (OPERATOR_PASS_THROUGH & status) {
                    continue;
                } else {
                    current_status = OPERATOR_ZERO;
                    break;
                }
            }
        }
    }
    world_root_on_Event(get_world_root(), event);

    // 需要一个状态来确定需要进入3d来处理
    if (current_status == OPERATOR_ZERO) {
    }
}

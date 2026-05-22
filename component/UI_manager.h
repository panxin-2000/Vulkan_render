//
// Created by 潘鑫 on 2026/5/22.
//

#ifndef HELLO_MAC_UI_MANAGER_H
#define HELLO_MAC_UI_MANAGER_H


#include "global_singleton.h"
#include "name_component.h"
#include "Rect_2D_component.h"
#include "scene_component.h"
#include "shader_component.h"


class UI_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = Logic_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           Logic_entt().emplace<Scene_Component>(instance);
                           Logic_entt().emplace<Name_component>(instance, "scene_root");
                           Logic_entt().emplace<VKR_shader_paths>(instance,
                                                                  "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                                                  "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                                                  "", "");
                           matrix_4x4 view;
                           identity_matrix_4x4(&view);
                           set_render_parameter(instance, "global_view_4x4", view);

                           matrix_4x4 projection;
                           UI_projection_4x4(&projection, 1280, 720);
                           set_render_parameter(instance, "global_projection_4x4", projection);

                           if (auto *scene_node = Logic_entt().try_get<Rect_2D_transform>(instance)) {
                               scene_node->set_bounding_box({0, 0},
                                                            {
                                                                static_cast<float>(get_win_WIDTH()),
                                                                static_cast<float>(get_win_HEIGHT())
                                                            });
                           }
                           // 在系统初始化时，给logic_render_data * 的类型都添加这个销毁前执行的函数
                           // g_entt().on_destroy<logic_render_data>().connect<&cleanup_logic_render_data>();
                           // 也可以在只移除 logic_render_data 时 触发，但是不同类型触发的顺序可能是随机的。
                       }
                      );

        return instance;
    }

private
:
    UI_scene_root() = default; // 禁用构造
};


static entt::entity &get_UI_scene_root() {
    return UI_scene_root::get();
}


inline std::vector<entt::entity> UI_stack_intersect(const Point_2 &current_position) {
    std::vector<entt::entity> return_value;
    const auto scene_root_node = get_UI_scene_root();
    return_value.push_back(scene_root_node);
    get_intersect_entity(return_value, scene_root_node, current_position);
    return return_value;
}

#endif //HELLO_MAC_UI_MANAGER_H

//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H


#include <scene_component.h>
#include "name_component.h"
#include "model_matrix.h"
#include "shader_component.h"


class model_transform {
    Point_3 zoom   = {1, 1, 1};
    Point_3 offset = {0, 0, 0};
    Quaternion rotate;
    AABB_centroid<Point_3> bounding_box_; // 每次都直接计算吧。

public:
    [[nodiscard]] Point_3 get_zoom() const {
        return zoom;
    }

    [[nodiscard]] Point_3 get_offset() const {
        return offset;
    }

    void set_bounding_box(const Point_3 min, const Point_3 max) {
        bounding_box_ = AABB_centroid<Point_3>(min, max);
    }


    static bool check_entity_intersect_point(entt::entity entity, const Point_2 &current_position) {
        if (auto *scene_node = g_entt().try_get<model_transform>(entity)) {
            // 下面这个3d部分是需要去写的，但是只能通过射线来进行检测了
            // if (intersect(scene_node->bounding_box_, current_position)) {
            // return true;
            // }
        }
        return false;
    }
};


class world_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = g_entt().create();;
        static std::once_flag flag;

        std::call_once(flag, []() {
                           g_entt().emplace<Scene_Component>(instance);
                           g_entt().emplace<Name_component>(instance, "scene_root");
                           g_entt().emplace<VKR_shader_paths>(instance,
                                                              "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                                              "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                                              "", "");
                           matrix_4x4 view;
                           identity_matrix_4x4(&view);
                           set_render_parameter(instance, "global_view_4x4", view);

                           matrix_4x4 projection;
                           identity_matrix_4x4(&projection);
                           set_render_parameter(instance, "global_projection_4x4", projection);

                           if (auto *scene_node = g_entt().try_get<model_transform>(instance)) {
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
    world_scene_root() = default; // 禁用构造
};


static entt::entity &get_world_root() {
    return world_scene_root::get();
}

#endif //HELLO_MAC_RENDER_COMPONENT_H

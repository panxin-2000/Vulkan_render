//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H


#include <scene_component.h>

#include "DirectXMath.h"
#include "name_component.h"
#include "model_matrix.h"
#include "render_proxy.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"


class alignas(16) model_transform {
public:
    DirectX::XMFLOAT4 rotate = {0, 0, 0, 1};
    Point_3 zoom             = {1, 1, 1};
    Point_3 offset           = {0, 0, 0};
    AABB_centroid<Point_3> bounding_box_; // 每次都直接计算吧。

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


void sfgh(Point_3 zoom, DirectX::XMFLOAT4 &rotate, Point_3 offset) {
    DirectX::XMVECTOR scale   = DirectX::XMVectorSet(zoom.x, zoom.y, zoom.z, 0.0f);       // 缩放
    DirectX::XMVECTOR rotQuat = DirectX::XMLoadFloat4(&rotate);                           // 旋转(四元数)
    DirectX::XMVECTOR pos     = DirectX::XMVectorSet(offset.x, offset.y, offset.z, 0.0f); // 平移

    // 2. 生成各自的变换矩阵
    DirectX::XMMATRIX mScale       = DirectX::XMMatrixScalingFromVector(scale);
    DirectX::XMMATRIX mRotation    = DirectX::XMMatrixRotationQuaternion(rotQuat);
    DirectX::XMMATRIX mTranslation = DirectX::XMMatrixTranslationFromVector(pos);

    DirectX::XMMATRIX modelMatrix = mScale * mRotation * mTranslation;
}


inline void update_object_offset() {
    const auto view = g_entt().view<Position_update_tag, std::shared_ptr<VKR_object_proxy>, model_transform>();
    // 包围盒发生了更新
    for (const auto it: view) {
        auto transform                = view.get<model_transform>(it);
        const DirectX::XMVECTOR scale =
                DirectX::XMVectorSet(transform.zoom.x, transform.zoom.y, transform.zoom.z, 0.0f);
        const DirectX::XMVECTOR rotQuat = DirectX::XMLoadFloat4(&transform.rotate);
        const DirectX::XMVECTOR pos = DirectX::XMVectorSet(transform.offset.x, transform.offset.y, transform.offset.z,
                                                           0.0f);

        // 2. 生成各自的变换矩阵
        const DirectX::XMMATRIX mScale       = DirectX::XMMatrixScalingFromVector(scale);
        const DirectX::XMMATRIX mRotation    = DirectX::XMMatrixRotationQuaternion(rotQuat);
        const DirectX::XMMATRIX mTranslation = DirectX::XMMatrixTranslationFromVector(pos);

        const DirectX::XMMATRIX modelMatrix = mScale * mRotation * mTranslation;


        set_render_parameter(it, "model_4x4", modelMatrix);

        auto result = set_render_push_constant_parameter(it, "model_4x4", view);

        auto lambda = [result](const std::shared_ptr<VKR_object_proxy> &proxy) {
            proxy->push_constants_address = result;
        };
        update_VKR_object_proxy(it, lambda);
        g_entt().remove<Position_update_tag>(it);
    }
}


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

                           const uint32_t WIDTH  = 1280; // 也是需要更改的
                           const uint32_t HEIGHT = 720;

                           // 1. 生成标准的右手系透视矩阵 (Z 范围 0 到 1)
                           const DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(
                                DirectX::XMConvertToRadians(45.0f),
                                (float) WIDTH / (float) HEIGHT,
                                0.1f,
                                1000.0f
                               );
                           DirectX::XMMATRIX flip_y     = DirectX::XMMatrixScaling(1.0f, -1.0f, 1.0f);
                           DirectX::XMMATRIX projection = proj * flip_y;
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

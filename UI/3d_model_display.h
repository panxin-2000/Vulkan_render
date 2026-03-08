//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_3D_MODEL_DISPLAY_H
#define HELLO_MAC_3D_MODEL_DISPLAY_H

#include "global_singleton.h"
#include "mesh_component.h"
#include "model_matrix.h"
#include "name_component.h"
#include "shader_component.h"
#include "base_element/point_3.h"

struct Suzanne_push_constant {
    matrix_4x4 projection;
    matrix_4x4 view;
    matrix_4x4 model[3];
    float lightPos[4]{0.0f, -10.0f, 10.0f, 0.0f};
    uint32_t selected{1};
    uint32_t selected8{1};
    uint32_t selected7{1};
    uint32_t selected6{1};
};

inline Suzanne_push_constant get_shader_data() {
    const uint32_t WIDTH  = 1280; // 也是需要更改的
    const uint32_t HEIGHT = 720;
    Suzanne_push_constant shaderData;
    Quaternion r;

    DirectX::XMVECTOR pos     = DirectX::XMVectorSet(0.0f, 0.0f, 6.0f, 0.0f); // 相机位置
    DirectX::XMVECTOR rotQuat = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 相机旋转四元数

    // 先求相机的世界矩阵 (S * R * T, 缩放通常为 1)
    DirectX::XMMATRIX camWorld = DirectX::XMMatrixRotationQuaternion(rotQuat) *
                                 DirectX::XMMatrixTranslationFromVector(pos);

    // View 矩阵就是相机世界矩阵的 逆矩阵
    DirectX::XMVECTOR det;
    DirectX::XMMATRIX viewMatrix_transpose = XMMatrixInverse(&det, camWorld);
    memcpy(&shaderData.view, &viewMatrix_transpose, sizeof(DirectX::XMMATRIX));


    // 1. 生成标准的右手系透视矩阵 (Z 范围 0 到 1)
    const DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovRH(
                                                                     DirectX::XMConvertToRadians(45.0f),
                                                                     (float) WIDTH / (float) HEIGHT,
                                                                     0.1f,
                                                                     1000.0f
                                                                    );
    DirectX::XMMATRIX flip_y               = DirectX::XMMatrixScaling(1.0f, -1.0f, 1.0f);
    DirectX::XMMATRIX projection_transpose = proj * flip_y;

    memcpy(&shaderData.projection, &projection_transpose, sizeof(DirectX::XMMATRIX));
    for (auto i = 0; i < 3; i++) {
        Point_3 instancePos{(float) (i - 1) * 4.0f, 0.0f, 0.0f};
        auto point = reinterpret_cast<float *>(&shaderData.model[i]);
        scale s;
        model_matrix_4x4(point, instancePos, r, s);
    }
    return shaderData;
}

inline void object_3d_model(const std::string &name, const std::string &mesh_path) {
    entt::entity entity_ = g_entt().create();
    g_entt().emplace<Name_component>(entity_, name);
    g_entt().emplace<VKR_shader_paths>(entity_,
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                       "", "");
    add_geometry_data(entity_, mesh_path);

    auto sdfgh = get_shader_data();
    set_render_parameter(entity_, "global_projection_4x4", sdfgh.projection);
    set_render_parameter(entity_, "global_view_4x4", sdfgh.view);
    set_render_parameter(entity_, "model_4x4", sdfgh.model[0]);
    g_entt().emplace_or_replace<add_to_render_tag>(entity_);
}


#endif //HELLO_MAC_3D_MODEL_DISPLAY_H

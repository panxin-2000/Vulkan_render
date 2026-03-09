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
#include "model_transform_component.h"

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


    const auto viewMatrix = view_matrix({0.0f, 0.0f, 6.0f}, Eigen::Quaternionf::Identity());
    memcpy(&shaderData.view, &viewMatrix, sizeof(viewMatrix));


    const auto projection = vulkan_projection(to_radians(45.0f),
                                              (float) WIDTH / (float) HEIGHT,
                                              0.1f,
                                              32.0f);


    memcpy(&shaderData.projection, &projection, sizeof(projection));
    for (auto i = 0; i < 3; i++) {
        Point_3 instancePos{(float) (i - 1) * 4.0f, 0.0f, 0.0f};
        auto point = reinterpret_cast<float *>(&shaderData.model[i]);
        scale s;
        model_matrix_4x4(point, instancePos, r, s);
    }
    return shaderData;
}

inline entt::entity object_3d_model(const std::string &name, const std::string &mesh_path, const Point_3 offset,
                                    const Eigen::Quaternionf &rotate = Eigen::Quaternionf::Identity()) {
    entt::entity entity_ = g_entt().create();
    g_entt().emplace<Name_component>(entity_, name);

    g_entt().emplace<VKR_shader_paths>(entity_,
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
                                       "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
                                       "", "");
    add_geometry_data(entity_, mesh_path);

    // 更新物体的模型矩阵
    g_entt().emplace<model_transform>(entity_, offset, rotate);
    const auto &transform  = g_entt().get<model_transform>(entity_);
    const auto modelMatrix = transform.update_model_matrix();
    set_render_parameter(entity_, "model_4x4", modelMatrix);

    world_root_add_child(entity_);

    g_entt().emplace_or_replace<add_to_render_tag>(entity_);
    return entity_;
}


#endif //HELLO_MAC_3D_MODEL_DISPLAY_H

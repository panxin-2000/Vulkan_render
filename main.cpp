/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <GLFW/glfw3.h>

#include <thread>
#include "render_thread/backend.h"
#include "vulkan_backend.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"
#include <iostream>
#include "descriptor_pool.h"
#include "sync_proxy_to_render_thread.h"
#include "update_push_constants_data.h"
#include "gltf_model/load_gltf_model.h"
#include "UI/3d_model_display.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


inline entt::entity add_render_pass(const std::string &name) {
    entt::entity entity = Logic_entt().create();

    Logic_entt().emplace<Name_component>(entity, name + "deferred_pass");

    Logic_entt().emplace<VKR_shader_paths>(entity,
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.vert.spv",
                                           "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.frag.spv",
                                           "", "");

    // 更新物体的模型矩阵

    world_root_add_child(entity);

    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    return entity;
}


VkSampler base_sample() {
    const auto &backend    = VK_backend::get();
    VkSampler colorSampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler{};
    sampler.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.maxAnisotropy = 1.0f;
    sampler.magFilter     = VK_FILTER_NEAREST;
    sampler.minFilter     = VK_FILTER_NEAREST;
    sampler.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler.addressModeV  = sampler.addressModeU;
    sampler.addressModeW  = sampler.addressModeU;
    sampler.mipLodBias    = 0.0f;
    sampler.maxAnisotropy = 1.0f;
    sampler.minLod        = 0.0f;
    sampler.maxLod        = 1.0f;
    sampler.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(backend.get_device(), &sampler, nullptr, &colorSampler));
    return colorSampler;
}


int main(int argc, char *argv[]) {
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    // std::cout << " UI_component.h:111  " << std::endl; // 是文件的路径就可以在clion中直接点击显示
    auto &backend = VK_backend::get();
    backend.engine_init(); // 必须单独调用，不能在 std::call_once 中 ，否则会死锁
    init_current_descriptor_pool();


    render_thread_start(backend);

    register_glfw(backend.get_window());

    UI_block("按钮1", 0, 0, 60, 60);
    UI_block("功能块", 0, 0, 50, 200);
    UI_block("按钮2", 0, 0, 145, 130);


    load_gltf_model("Damaged Helmet", "assets/DamagedHelmet.gltf");

    // 3d 模型
    {
        auto entity = object_3d_model("blender Suzanne -3", "assets/suzanne.obj", {-3.0f, 0.0f, 0.0f});
        set_render_parameter(entity, "samplerColor", "assets/suzanne0.ktx");
    } {
        auto entity = object_3d_model("blender Suzanne +3", "assets/suzanne.obj", {3.0f, 0.0f, 0.0f});
        set_render_parameter(entity, "samplerColor", "assets/suzanne1.ktx");
    }
    const auto sampler = base_sample(); {
        const auto entity                  = add_render_pass("blank");
        Texture_parameter position_texture = {
            .image       = backend.G_buffer_Position_images_.at(1),
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> position = position_texture;
        Texture_parameter normal_texture          = {
            .image       = backend.g_buffer_Normal_images_.at(1),
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> normal = normal_texture;
        Texture_parameter baseColor_texture     = {
            .image       = backend.G_buffer_BaseColor_images_.at(1),
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> baseColor = baseColor_texture;

        Point_2 temp_value = {2.0, 2.0};
        set_push_constant_parameter(entity, "frag_scale", temp_value);

        // 下面三个只能选择一个显示，问题应该再下面的函数中，而不是frag shader中
        set_render_parameter(entity, "samplerPosition", position);
        set_render_parameter(entity, "samplerNormal", normal);
        set_render_parameter(entity, "samplerBaseColor", baseColor);
        auto temp_ptr          = create_SSBO_buffer(1024 * 5);
        float color[16]        = {1.0f, 0.0f, 0.0f, 1.0f};
        auto mem_copy_function = [color](void *dst) {
            memcpy(dst, color, sizeof(color));
        };
        copy_mem_from_cpu_to_gpu(temp_ptr, mem_copy_function);
        set_render_parameter(entity, "light_buffer", temp_ptr);
    }

    // Render loop
    while (!glfwWindowShouldClose(backend.get_window())) {
        glfwWaitEvents();
        if (GLFW_TRUE == glfwWindowShouldClose(backend.get_window())) {
            break;
        }
        glfwPollEvents();  // Event polling
        deal_glfw_event(); // 统一分发执行
        clean_render_entity();
        sync_render_data_to_render_thread();

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    Logic_entt().clear(); // 必须先清理， root entity 会占有一部分资源，需要先清理

    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->destroy_buffer();
    vkDestroySampler(backend.get_device(), sampler, nullptr);

    backend.engine_destroy();
    backend.destroy();
}


void test_projection_matrix() {
    auto entity = object_3d_model("triangle", "", {0.0f, 0.0f, 0.0f});
    add_geometry_data(entity, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f});
}

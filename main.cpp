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
#include "descriptor_pool.h"
#include "sync_proxy_to_render_thread.h"
#include "update_push_constants_data.h"
#include "vk_render_to_image.h"
#include "vulkan_sample.h"
#include "ccd/ccd.h"
#include "UI/3d_model_display.h"

void register_glfw(GLFWwindow *window);

void deal_glfw_event();


inline entt::entity add_render_pass(const std::string &name) {
    entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Proxy_entity>(entity, Render_entt().create());

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

void my_support(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *vec) {
    // 1. 强制转回你的连续内存容器
    const auto &mesh = *static_cast<const std::vector<Point_3> *>(obj);

    float max_dot = -FLT_MAX;
    int best_idx  = 0;

    // 2. 直接在连续内存上进行点积（性能极高）
    for (size_t i = 0; i < mesh.size(); ++i) {
        float dot = mesh[i].x * dir->v[0] + mesh[i].y * dir->v[1] + mesh[i].z * dir->v[2];
        if (dot > max_dot) {
            max_dot  = dot;
            best_idx = i;
        }
    }

    // 3. 将结果写回给 libccd
    vec->v[0] = mesh[best_idx].x;
    vec->v[1] = mesh[best_idx].y;
    vec->v[2] = mesh[best_idx].z;
}


void test_lib_ccd() {
    ccd_t ccd;
    CCD_INIT(&ccd);

    // 设置回调函数
    ccd.support1       = my_support;
    ccd.support2       = my_support;
    ccd.max_iterations = 100;    // 迭代次数限制
    ccd.epa_tolerance  = 0.0001; // maximal tolerance fro EPA part

    std::vector<Point_3> meshA = {{0, 0, 0}, {2, 0, 0}, {0, 2, 0}};
    std::vector<Point_3> meshB = {{0, 3, 3}, {2, 3, 3}, {0, 6, 3}};

    ccd_vec3_t sep;

    ccd_real_t depth;
    ccd_vec3_t dir, pos;

    // 直接传入 vector 的地址即可
    int intersect = ccdGJKPenetration(&meshA, &meshB, &ccd, &depth, &dir, &pos);

    if (intersect) {
        // 发生了碰撞！
    }
}

int main(int argc, char *argv[]) {
    test_lib_ccd();
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");
    // std::cout << " UI_component.h:111  " << std::endl; // 是文件的路径就可以在clion中直接点击显示
    auto &backend = VK_backend::get();
    backend.engine_init(); // 必须单独调用，不能在 std::call_once 中 ，否则会死锁
    init_current_descriptor_pool();


    // UI 部分有些细节做的不到位，但是还是全黑的，且没有警告提示了
    UI_block("按钮1", 0, 0, 60, 60);
    UI_block("功能块", 0, 0, 50, 200);
    UI_block("按钮2", 0, 0, 145, 130);


    // load_gltf_model("sky box", "assets/Box.gltf");
    // load_gltf_model("Damaged Helmet", "assets/DamagedHelmet.gltf");

    // 天空盒
    {
        auto entity                                     = add_sky_box("skybox");
        auto texture                                    = create_skybox_texture_all("");
        std::optional<Texture_parameter> sampler_skybox = texture;
        set_render_parameter(entity, "sampler_skybox", sampler_skybox);
        logic_update_add_tag<skybox_tag>(entity);

        // 还需再增加一个特殊的标记，用于最后绘制，UI前，所有3D 完成后
    }
    // 3d 模型
    {
        auto entity    = object_3d_model("blender Suzanne -3", "assets/suzanne.obj", {-3.0f, 0.0f, 0.0f});
        auto texture   = create_textures_to_gpu(backend, "assets/suzanne0.ktx");
        uint32_t index = add_bindless_uniform_sampler2D("assets/suzanne0.ktx", texture);
        set_render_parameter(entity, "samplerColor", index);
        logic_update_add_tag<opacity_tag>(entity);
    } {
        auto entity  = object_3d_model("blender Suzanne +3", "assets/suzanne.obj", {3.0f, 0.0f, 0.0f});
        auto texture = create_textures_to_gpu(backend, "assets/suzanne1.ktx");
        auto index   = add_bindless_uniform_sampler2D("assets/suzanne1.ktx", texture);
        set_render_parameter(entity, "samplerColor", index);
        logic_update_add_tag<opacity_tag>(entity);
    }

    render_thread_start(backend);

    register_glfw(backend.get_window());

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

        // vk_render_GPU::instance().one_cycle(backend);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    Logic_entt().clear(); // 必须先清理， root entity 会占有一部分资源，需要先清理

    // vk_render_GPU::instance().exit_and_clean(backend);
    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->destroy_buffer();

    backend.engine_destroy();
    backend.destroy();
}


void test_projection_matrix() {
    auto entity = object_3d_model("triangle", "", {0.0f, 0.0f, 0.0f});
    add_geometry_data(entity, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f});
}


void add_deferred_pass(void) {
    auto &backend      = VK_backend::get();
    const auto sampler = base_sample(); {
        const auto entity = add_render_pass("blank");
        logic_update_add_tag<deferred_pass_tag>(entity);

        Texture_parameter position_texture = {
            .image       = backend.G_buffer_Position_images_.at(0), // 之前的差一帧的会出现绿色的问题在这里
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> position = position_texture;
        Texture_parameter normal_texture          = {
            .image       = backend.g_buffer_Normal_images_.at(0), // 之前的差一帧的会出现绿色的问题在这里
            .sampler     = sampler,
            .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        };
        std::optional<Texture_parameter> normal = normal_texture;
        Texture_parameter baseColor_texture     = {
            .image       = backend.G_buffer_BaseColor_images_.at(0), // 之前的差一帧的会出现绿色的问题在这里
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
}

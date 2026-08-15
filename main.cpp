/* Copyright (c) 2025-2026, Sascha Willems
 * SPDX-License-Identifier: MIT
 */

#include <SDL3/SDL.h>

#include <thread>
#include "render_thread/backend.h"
#include "vulkan_backend.h"
#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"

#include "global_singleton.h"
#include "descriptor_pool.h"
#include "device_input_event_deal.h"
#include "earcut.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "load_gltf_model.h"
#include "nano_vdb_model.h"
#include "render/render_common/PBR_component.h"
#include "sync_proxy_to_render_thread.h"
#include "UI_manager.h"
#include "update_push_constants_data.h"
#include "vk_render_to_image.h"
#include "vulkan_sample.h"
#include "ccd/ccd.h"
#include "object_model/3d_model_display.h"
#include "UI/UI_imgui.h"
#include "UI/UI_text.h"
#include "imgui.h"
#include "skybox.h"


#include "spherical_harmonics.h"
#include "spherical_SH.h"
#include "world_scene_root.h"


Uint32 SDLCALL MyTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval) {
    const char *message = (const char *) userdata;
    printf("定时器触发! 消息: %s, 间隔: %u ms\n", message, interval);

    // 返回 interval 表示持续循环触发；返回 0 表示单次触发后销毁
    return interval;
}

int main(int argc, char *argv[]) {
    LOG_INFO(g_log(), "Hello from {}!", "Quill v11.0.2");

    auto &backend = VK_backend::instance();
    backend.create();
    auto &engine = Engine::instance();
    engine.create();
    auto world_root = get_world_root();
    auto UI_root    = get_UI_scene_root();
    // 需要确定启动的顺序
    render_thread_start(backend);



    // UI 部分有些细节做的不到位，但是还是全黑的，且没有警告提示了
    // UI_block("按钮1", 0, 0, 60, 60);
    // UI_block("功能块", 0, 0, 50, 200);
    // UI_block("按钮2", 0, 0, 145, 130);
    //

    add_skybox_entity();
    // add_manifold_entity();

    // add_simple_computer_buffer_write();
    // add_volume_pass("/Users/panxin/CLionProjects/hello_mac/Sphere.nvdb");
    // add_volume_pass("/Users/panxin/CLionProjects/hello_mac/assets/SmallCampfireVDB/smallCampfire/smallCampfireVDB/smallCampfire_0000.vdb");
    // add_volume_pass("./assets/CloudPackVDB/CloudPack/CloudPackVDB/cloud_01_variant_0000.vdb");


    // {
    // auto entity = UI_text("AbcgoyQj", 200, 200, 500, 500);
    // }
    object_3d_model("box", {{1, 1, 1}, {2, 2, 2}});
    object_line("line");
    object_line_old(" old");
    // object_3d_model("box", {{510, 510, 500}, {520, 520, 520}});
    // auto entity = load_gltf_model("Sponza",
    //                               "/Users/panxin/file_sync/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");

    // load_gltf_model("DamagedHelmet.gltf",
    //                 "/Users/panxin/file_sync/glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");


    // Setup Dear ImGui context


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui_ImplSDL3_InitForVulkan(backend.get_window());


    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("/Users/panxin/CLionProjects/hello_mac/imgui/misc/fonts/Cousine-Regular.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    io.Fonts->Build();
    // 字体的内容存储在了alpha通道中

    // 扩展和层并不在意添加的顺序，能否在一开始就将需要的层和扩展添加了，之后根据具体的实现判断是否能获得，能获得就添加。

    // Setup Platform/Renderer backends
    // getSingleInstance() ···等申请的内容，都是是在 SetupVulkan  中做完的，之后绘制的时候绑定提交会调用init_info中的内容（或者说指向）
    // ImGui_ImplGlfw_InitForVulkan(backend.get_window(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    ImGui_ImplVulkan_Init(&init_info);


    bool show_demo_window    = false;
    bool show_another_window = false;
    ImVec4 clear_color       = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    auto imgui_entity        = create_imgui_entity("imgui", nullptr);
    Point_3 world_light_pos(0.0f, 1.0f, 0.0f);

    // Render loop
    float run_time = 0;
    bool done      = false;
    FrameRate_measure framerate_measure(60.0f);
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        // // 只有当 ImGui 不需要鼠标时，主程序才响应鼠标事件（如点击选中 3D 物体）
        // if (!io.WantCaptureMouse) {
        //     ProcessMainApplicationMouse(mouseData);
        // }
        // // 只有当 ImGui 不需要键盘时，主程序才响应键盘事件（如 WASD 移动）
        // if (!io.WantCaptureKeyboard) {
        //     ProcessMainApplicationKeyboard(keyboardData);
        // }


        framerate_measure.begin_frame();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
                base_event_dealing(event);
            }
            if (event.type == SDL_EVENT_DROP_FILE) {
                SDL_Log("File: %s", event.drop.data); // 获取路径
                std::filesystem::path filePath = event.drop.data;
                const auto entity              = load_gltf_model(filePath.stem().string(), filePath);
            }
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID ==
                SDL_GetWindowID(backend.get_window()))
                done = true;
        }

        Logic_entt().emplace_or_replace<Camera_dirty>(get_world_root());
        imgui_draw_new_frame(imgui_entity, show_demo_window, show_another_window, clear_color, world_light_pos);
        clean_render_entity();
        run_time = run_time + 1.0f / 60.0f;
        sync_render_data_to_render_thread(run_time);
        framerate_measure.end_frame();
    }


    // IM_ASSERT_USER_ERROR(g.IO.BackendPlatformUserData == NULL, "Forgot to shutdown Platform backend?");
    // IM_ASSERT_USER_ERROR(g.IO.BackendRendererUserData == NULL, "Forgot to shutdown Renderer backend?");
    // 上面两个需要清理
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // free_bindless_uniform_sampler2D("white_color_texture"); 不用时需要手动清理，但是world 实体销毁之后也会自动清理
    Logic_entt().clear(); // 必须先清理， root entity 会占有一部分资源，需要先清理

    // vk_render_GPU::instance().exit_and_clean(backend);
    render_thread_stop_and_wait();

    // 全局的 push_constants 的 buffer ,最后在这里销毁稍微有点不太好。
    auto &buffer = get_uniform_buffer();
    buffer->destroy_buffer();

    engine.destroy();
    backend.destroy();
}


void test_projection_matrix() {
    auto entity = object_3d_model("triangle", "", {0.0f, 0.0f, 0.0f});
    add_triangle_geometry(entity, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f});
}

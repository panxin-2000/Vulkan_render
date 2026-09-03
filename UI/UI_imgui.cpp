//
// Created by 潘鑫 on 2026/5/28.
//

#include "UI_imgui.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "load_gltf_model.h"
#include "move_speed.h"
#include "name_component.h"
#include "render_common/render_state.h"
#include "scene_component.h"
#include "select_component.h"
#include "shader_component.h"
#include "time_measure.h"
#include "transform_component.h"
#include "UI_manager.h"
#include "world_scene_root.h"


struct ImGui_ImplVulkan_FrameRenderBuffers {
    VkDeviceMemory VertexBufferMemory;
    VkDeviceMemory IndexBufferMemory;
    VkDeviceSize VertexBufferSize;
    VkDeviceSize IndexBufferSize;
    VkBuffer VertexBuffer;
    VkBuffer IndexBuffer;
};

struct ImGui_ImplVulkan_WindowRenderBuffers {
    uint32_t Index;
    uint32_t Count;
    ImGui_ImplVulkan_FrameRenderBuffers *FrameRenderBuffers;
};

// Vulkan data
struct ImGui_ImplVulkan_Data {
    ImGui_ImplVulkan_InitInfo VulkanInitInfo;
    VkDeviceSize BufferMemoryAlignment;
    VkPipelineCreateFlags PipelineCreateFlags;
    VkDescriptorSetLayout DescriptorSetLayout;
    VkPipelineLayout PipelineLayout;
    VkPipeline Pipeline;
    VkShaderModule ShaderModuleVert;
    VkShaderModule ShaderModuleFrag;

    // Font data
    VkSampler FontSampler;
    VkDeviceMemory FontMemory;
    VkImage FontImage;
    VkImageView FontView;
    VkDescriptorSet FontDescriptorSet;
    VkCommandPool FontCommandPool;
    VkCommandBuffer FontCommandBuffer;

    // Render buffers for main window
    ImGui_ImplVulkan_WindowRenderBuffers MainWindowRenderBuffers;

    ImGui_ImplVulkan_Data() {
        memset((void *) this, 0, sizeof(*this));
        BufferMemoryAlignment = 256;
    }
};

bool ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo *info) {
    // IM_ASSERT(g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");

    ImGuiIO &io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    ImGui_ImplVulkan_Data *bd  = IM_NEW(ImGui_ImplVulkan_Data)();
    io.BackendRendererUserData = (void *) bd;
    io.BackendRendererName     = "imgui_impl_vulkan";
    io.BackendFlags            |= ImGuiBackendFlags_RendererHasVtxOffset;
    // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.

    return true;
}


void update_imgui_geometry(const entt::entity entity, ImDrawData *draw_data) {
    int fb_width  = (int) (draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int) (draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    // 还是有点小问题的，已修改
    const auto vertices = std::make_shared<std::vector<Vertex_imgui> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();     //  2   * 6 = 12

    if (draw_data != nullptr && draw_data->TotalVtxCount > 0) {
        // Create or resize the vertex/index buffers
        vertices->resize(draw_data->TotalVtxCount);
        indices->resize(draw_data->TotalIdxCount);
        auto vtx_dst = (vertices->data());
        auto idx_dst = (indices->data());
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
        clean_geometry_data(entity);
        add_geometry_data(entity, vertices, indices);
        auto mesh = get_VKR_mesh(entity);
        logic_update_proxy(entity, mesh);
        auto primitives = create_primitives(entity);
        std::vector<VKR_Render_state> render_states;
        if (!primitives.empty()) {
            VKR_Primitive vkr_primitive = primitives.at(0);
            VKR_Render_state render_state;
            primitives.clear();
            // index_count = 2136
            // first_      = 12480
            // Will project scissor/clipping rectangles into framebuffer space
            ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
            ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

            int global_vtx_offset = 0;
            int global_idx_offset = 0;
            for (const ImDrawList *draw_list: draw_data->CmdLists) {
                for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++) {
                    const ImDrawCmd *pcmd = &draw_list->CmdBuffer[cmd_i];
                    if (pcmd->UserCallback != nullptr) {
                        // User callback, registered via ImDrawList::AddCallback()
                    } else {
                        // Project scissor/clipping rectangles into framebuffer space
                        ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                        (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                        ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                        (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                        // Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
                        if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
                        if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
                        if (clip_max.x > (float) fb_width) { clip_max.x = (float) fb_width; }
                        if (clip_max.y > (float) fb_height) { clip_max.y = (float) fb_height; }
                        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                            continue;
                        // Apply scissor/clipping rectangle
                        render_state.viewport.x            = 0;
                        render_state.viewport.y            = 0;
                        render_state.viewport.width        = fb_width;
                        render_state.viewport.height       = fb_height;
                        render_state.viewport.minDepth     = 0.0f;
                        render_state.viewport.maxDepth     = 1.0f;
                        render_state.scissor.offset.x      = (int32_t) (clip_min.x);
                        render_state.scissor.offset.y      = (int32_t) (clip_min.y);
                        render_state.scissor.extent.width  = (uint32_t) (clip_max.x - clip_min.x);
                        render_state.scissor.extent.height = (uint32_t) (clip_max.y - clip_min.y);
                        render_state.set_front_face(VK_FRONT_FACE_COUNTER_CLOCKWISE);
                        render_state.set_VkCullModeFlags(VK_CULL_MODE_NONE);

                        vkr_primitive.indexCount    = pcmd->ElemCount;
                        vkr_primitive.instanceCount = 1;
                        vkr_primitive.firstIndex    = pcmd->IdxOffset + global_idx_offset;
                        vkr_primitive.vertexOffset  = pcmd->VtxOffset + global_vtx_offset;
                        vkr_primitive.firstInstance = 0;
                        primitives.emplace_back(vkr_primitive);
                        render_states.push_back(render_state);
                    }
                }
                global_idx_offset += draw_list->IdxBuffer.Size;
                global_vtx_offset += draw_list->VtxBuffer.Size;
            }
        }

        float scale[2];
        scale[0] = 2.0f / draw_data->DisplaySize.x; // Scale
        scale[1] = 2.0f / draw_data->DisplaySize.y;
        float translate[2];
        translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0]; // Translate
        translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];

        set_push_constant_parameter(entity, "uScale", scale);
        set_push_constant_parameter(entity, "uTranslate", translate);

        logic_update_proxy(entity, primitives);
        logic_update_proxy(entity, render_states);
    }
}

entt::entity create_imgui_entity(const std::string &name, ImDrawData *draw_data) {
    const entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Name_component>(entity, "imgui ui");

    logic_create_proxy(entity);

    add_shader(entity, "2D/imgui", "2D/imgui", "", "");


    // 还是有点小问题的，已修改
    const auto vertices = std::make_shared<std::vector<Vertex_imgui> >(); //  32  * 4 = 128
    const auto indices  = std::make_shared<std::vector<uint16_t> >();     //  2   * 6 = 12

    if (draw_data != nullptr && draw_data->TotalVtxCount > 0) {
        // Create or resize the vertex/index buffers
        vertices->resize(draw_data->TotalVtxCount);
        indices->resize(draw_data->TotalIdxCount);
        auto vtx_dst = (vertices->data());
        auto idx_dst = (indices->data());
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
        add_geometry_data(entity, vertices, indices);
        // 应该只是几何数据对了， imgui 还是分了好几个批次去绘制 不同的 内容，还有 不同的 裁剪窗口
    }
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_add_tag<imgui_draw>(entity);
    UI_root_add_child(entity);
    return entity;

    // 需要添加一个的特殊的 imgui 的 标记
}

static ImGui_ImplVulkan_Data *ImGui_ImplVulkan_GetBackendData() {
    return ImGui::GetCurrentContext() ? (ImGui_ImplVulkan_Data *) ImGui::GetIO().BackendRendererUserData : nullptr;
}


void ImGui_ImplVulkan_Shutdown() {
    ImGui_ImplVulkan_Data *bd = ImGui_ImplVulkan_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO &io = ImGui::GetIO();

    // ImGui_ImplVulkan_DestroyDeviceObjects();
    io.BackendRendererName     = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags            &= ~ImGuiBackendFlags_RendererHasVtxOffset;
    IM_DELETE(bd);
}


bool ImGui_ImplVulkan_CreateFontsTexture(const entt::entity entity) {
    ImGuiIO &io = ImGui::GetIO();
    VkResult err;

    // Destroy existing texture (if any)


    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width * height * 4 * sizeof(char);
    Picture_parameters picture_parameters;
    picture_parameters.image_data            = pixels;
    picture_parameters.width                 = width;
    picture_parameters.height                = height;
    picture_parameters.channels              = 4;
    std::optional<Texture_parameter> texture = create_2d_texture(picture_parameters);

    set_render_parameter(entity, "sTexture", texture);
    // 这里就是看应该如何上传的时候了
    // imgui 是每帧都更新字体贴图吗？
    // 不是，只是最开始，之后的时候不会了，如果碰见没有的字，会用一个方块来替代


    return true;
}

void ImGui_ImplVulkan_NewFrame(const entt::entity entity) {
    ImGui_ImplVulkan_Data *bd = ImGui_ImplVulkan_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplVulkan_Init()?");

    if (!bd->FontDescriptorSet) {
        ImGui_ImplVulkan_CreateFontsTexture(entity);
        bd->FontDescriptorSet = (VkDescriptorSet) 1;
    }
}

void display_tree(entt::entity entity) {
    static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_DrawLinesFull;
    auto name                            = Logic_entt().get<Name_component>(entity);
    if (ImGui::TreeNodeEx(name.name_.c_str(), base_flags)) {
        // ImGui::Text("display");
        // ImGui::SameLine();
        // if (ImGui::SmallButton("button")) {
        // }
        if (const auto select = Logic_entt().try_get<select_component>(entity)) {
            ImGui::Checkbox("select", &select->selected);
        }
        if (const auto select = Logic_entt().try_get<load_material>(entity)) {
            if (ImGui::Checkbox("load_material", &select->selected)) {
                if (select->selected == false) {
                } else {
                    load_gltf_material_separate(entity);
                }
            }
        }


        if (const auto move_speed = Logic_entt().try_get<Move_speed>(entity)) {
            ImGui::DragFloat("speed", &move_speed->speed);
        }

        if (auto transform = Logic_entt().try_get<Transform>(entity)) {
            if (ImGui::DragFloat3("position", ((float *) transform) + 4 + 3)) {
                add_recursion_function_to_itself_children(entity, set_transform_dirty);
            }
        }


        // if (ImGui::Checkbox("mesh", &show_another_window)) {
        // }
        // if (ImGui::Checkbox("material", &show_another_window)) {
        // }
        auto &scene_component = Logic_entt().get<Scene_Component>(entity);
        for (uint32_t i = 0; i < scene_component.children_.size(); i++) {
            if (i == 0)
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushID(i);
            display_tree(scene_component.children_[i]);
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
}

entt::entity imgui_draw_new_frame(const entt::entity entity,
                                  bool &show_demo_window,
                                  bool &show_another_window,
                                  ImVec4 &clear_color,
                                  Point_3 &world_light_pos) {
    // Start the Dear ImGui frame
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplVulkan_NewFrame(entity);
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f                = 0.0f;
        static int counter            = 0;
        ImGuiWindowFlags window_flags = 0;
        window_flags                  |= ImGuiWindowFlags_NoMove;
        bool open                     = true;
        ImGui::Begin("Hello, world!", &open, window_flags);
        // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);              // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

        ImGuiIO &io = ImGui::GetIO();
        Engine::instance().get_global_parameters().set_mouse_position(io.MousePos.x, io.MousePos.y);
        ImGui::Text("MousePos: %.2f, %.2f", io.MousePos.x, io.MousePos.y);
        ImGui::SliderFloat("aperture",
                           &Engine::instance().get_global_parameters().get_aperture(), 0.95f, 22.0f);
        ImGui::SliderFloat("sensor_width",
                           &Engine::instance().get_global_parameters().get_sensor_width(), 0.0f, 1.0f);
        ImGui::SliderFloat("focal_length",
                           &Engine::instance().get_global_parameters().get_focal_length(), 0.0120f, 2.0f);
        ImGui::SliderFloat("maxBlurPixels",
                           &Engine::instance().get_global_parameters().get_maxBlurPixels(), 0.0f, 1000.0f);

        ImGui::SliderFloat("fog_start",
                           &Engine::instance().get_global_parameters().get_fog_start(), 0.0f, 1000.0f);
        ImGui::SliderFloat("fog_end",
                           &Engine::instance().get_global_parameters().get_fog_end(), 0.0f, 1000.0f);
        ImGui::SliderFloat("fog_density",
                           &Engine::instance().get_global_parameters().get_fog_density(), 0.0f, 1.0f);
        ImGui::SliderInt("fog_type",
                         (int *) &Engine::instance().get_global_parameters().get_fog_type(), 0, 2);
        ImGui::SliderFloat3("fog_color",
                            Engine::instance().get_global_parameters().get_fog_color().data(), 0.0f, 1.0f);
        ImGui::SliderFloat("camera_vignette_intensity",
                           &Engine::instance().get_global_parameters().get_camera_vignette_intensity(), 0.0f,
                           1.0f);
        ImGui::SliderFloat("camera_vignette_smoothness",
                           &Engine::instance().get_global_parameters().get_camera_vignette_smoothness(), 0.0f,
                           1.0f);
        ImGui::SliderFloat("film_grain_intensity",
                           &Engine::instance().get_global_parameters().get_film_grain_intensity(), 0.0f,
                           1.0f);
        (ImGui::SliderFloat3("light pos x y z", &world_light_pos.x, -1.0f, 1.0f)); {
            Engine::instance().get_global_parameters().set_sun_light({
                                                                         world_light_pos.x, world_light_pos.y,
                                                                         world_light_pos.z
                                                                     });
        }

        //
        {
            bool bistro = false;
            if (ImGui::Checkbox("bistro.gltf", &bistro)) {
                if (bistro == true) {
                    ScopedTimer temp("bistro.gltf"); // 56.89 s
                    load_gltf_model("bistro.gltf", "/Users/panxin/file_sync/bistro.gltf");
                }
            }
        } {
            bool bistro = false;
            if (ImGui::Checkbox("Sponza.gltf", &bistro)) {
                if (bistro == true) {
                    load_gltf_model("Sponza.gltf",
                                    "/Users/panxin/file_sync/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");
                }
            }
        } {
            bool bistro = false;
            if (ImGui::Checkbox("DamagedHelmet.gltf", &bistro)) {
                if (bistro == true) {
                    load_gltf_model("DamagedHelmet.gltf",
                                    "/Users/panxin/file_sync/glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");
                }
            }
        } {
            bool bistro = false;
            if (ImGui::Checkbox("fox.gltf", &bistro)) {
                if (bistro == true) {
                    load_gltf_model("fox.gltf",
                                    "/Users/panxin/file_sync/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf",
                                    {100, 0, 0});
                }
            }
        }


        ImGui::Text("Logic  thread average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        auto value = Engine::instance().get_framerate();
        ImGui::Text("Render thread average %.3f ms/frame (%d FPS)", 1000.0f / value, value);


        display_tree(get_UI_scene_root());
        display_tree(get_world_root());

        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
        ImGui::Begin("Another Window", &show_another_window);
        // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

    // Rendering
    ImGui::Render();

    ImDrawData *draw_data   = ImGui::GetDrawData(); //这里也是获取数据，之后再去拿去渲染。最后的数据是什么呢？
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized) {
        update_imgui_geometry(entity, draw_data);
    }

    return entity;
}

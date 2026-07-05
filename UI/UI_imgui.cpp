//
// Created by 潘鑫 on 2026/5/28.
//

#include "UI_imgui.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "name_component.h"
#include "scene_component.h"
#include "shader_component.h"


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
        add_geometry_data(entity, vertices, indices);
        auto [mesh , primitives] = get_VKR_mesh(entity);
        logic_update_proxy(entity, mesh);
        if (!primitives.empty()) {
            VKR_Primitive vkr_primitive = primitives.at(0);
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
                        vkr_primitive.scissor.offset.x      = (int32_t) (clip_min.x);
                        vkr_primitive.scissor.offset.y      = (int32_t) (clip_min.y);
                        vkr_primitive.scissor.extent.width  = (uint32_t) (clip_max.x - clip_min.x);
                        vkr_primitive.scissor.extent.height = (uint32_t) (clip_max.y - clip_min.y);
                        vkr_primitive.indexed_command.indexCount    = pcmd->ElemCount;
                        vkr_primitive.indexed_command.instanceCount = 1;
                        vkr_primitive.indexed_command.firstIndex    = pcmd->IdxOffset + global_idx_offset;
                        vkr_primitive.indexed_command.vertexOffset  = pcmd->VtxOffset + global_vtx_offset;
                        vkr_primitive.indexed_command.firstInstance = 0;
                        primitives.emplace_back(vkr_primitive);
                    }
                }
                global_idx_offset += draw_list->IdxBuffer.Size;
                global_vtx_offset += draw_list->VtxBuffer.Size;
            }
        }
        logic_update_proxy(entity, primitives);
        VkRect2D scissor = {{0, 0}, {(uint32_t) fb_width, (uint32_t) fb_height}};
        // vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }
}

entt::entity create_imgui_entity(const std::string &name, ImDrawData *draw_data) {
    const entt::entity entity = Logic_entt().create();
    Logic_entt().emplace<Name_component>(entity, "imgui ui");

    logic_create_proxy(entity);
    add_shader(entity,
               "/Users/panxin/CLionProjects/hello_mac/render/shader/imgui.vert.spv",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/imgui.frag.spv",
               "", "");


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
    Logic_entt().emplace_or_replace<add_to_render_tag>(entity);
    logic_update_add_tag<imgui_draw>(entity);
    float scale[2];
    scale[0] = 2.0f / 1280.f;
    scale[1] = 2.0f / 720;
    float translate[2];
    translate[0] = -1.0f - 0.0f * scale[0];
    translate[1] = -1.0f - 0.0f * scale[1];

    set_push_constant_parameter(entity, "uScale", scale);
    set_push_constant_parameter(entity, "uTranslate", translate);
    scene_root_add_child(entity);
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

entt::entity imgui_draw_new_frame(const entt::entity entity,
                                  bool &show_demo_window,
                                  bool &show_another_window,
                                  ImVec4 &clear_color) {
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplVulkan_NewFrame(entity);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f     = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);              // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))
            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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

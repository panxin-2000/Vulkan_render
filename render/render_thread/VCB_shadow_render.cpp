//
// Created by 潘鑫 on 2026/8/13.
//


#include "name_component.h"
#include "render_proxy.h"
#include "../engine.h"
#include "VCB_vulkan_command_buffer.h"


void VCB::CSM_pass(Engine &engine, const VKR_image_ptr &depth_shadow_image) {
    // 平行光 也是需要 视锥裁剪
    // 阴影的 pass
    // {
    //     // g_buffer_image_indices 这是需要看看怎么传递进入其中
    //     const auto view = Render_entt().view<shadow_pass_tag>();
    //     if (!view.empty()) {
    //         vcb.begin_shadow_pass(depth_image);

    // 中间需要添加 被光 照 到的物体，能产生阴影的物体
    // 这里的时候发生了一点改变，为什么呢？ 单个 mesh 需要多个不同的 render pass
    // 这也意味着 需要 它 开始需要多个 shader 了
    // 然后的问题就是，多个 shader 中，哪些是共同的部分，哪些不是？
    // 这里开始需要什么了呢？ 光，灯光是需要的部分 ，物体在世界空间的位姿也是需要的部分，
    // 不同的是输出的内容，以及输出的内容在什么时候再次被需要
    // model_matrix 是 在多个 shader 都保持不变的值。

    // 拿到z值，并反推出在世界空间中的位置需要 view 和 projection 的逆矩阵
    // 每个灯光，都需要其 view 和 projection 矩阵
    // 这个时候，其实应该是可以考虑关于遮挡部分的内容了，比如，
    // 计算平头截体，然后将不在其中的物体给排除出去。
    // 这个时候需要什么呢？ 物体的包围盒，model ,之后 再与 平头截体进行相交的判断
    // 之后再是什么呢？ 看看如何将这部分的计算放到GPU中计算


    begin_rendering_depth_attachment(depth_shadow_image,
                                         VK_ATTACHMENT_LOAD_OP_CLEAR);
    auto view = Render_entt().view<opacity_tag, GPU_frustum_cull, Name_component, VKR_shader_paths>();
    for (const auto entity: view) {
        auto command_calculate = Render_entt().get<GPU_frustum_cull>(entity);
        auto name              = Render_entt().get<Name_component>(entity);
        auto shader_path       = Render_entt().get<VKR_shader_paths>(entity);
        shader_path.clear_define_macro();
        shader_path.depthAttachmentFormat_   = VK_FORMAT_D32_SFLOAT;
        shader_path.stencilAttachmentFormat_ = VK_FORMAT_UNDEFINED;
        shader_path.add_define_macro("PASS_SHADOW_MAP", 1);
        const auto &shader_data_ref =
                engine.get_shader_manager().find(shader_path);
        bind_pipeline_update_parameter(entity, shader_data_ref);
        // 这里就需要看看怎么push
        for (uint i = 0; i < 4; ++i) {
            VkClearAttachment clearAttachment{};
            clearAttachment.aspectMask              = VK_IMAGE_ASPECT_DEPTH_BIT;
            clearAttachment.clearValue.depthStencil = {0.0f, 0}; // 刷成最远

            VkClearRect clearRect{};
            clearRect.rect.offset    = {0, 0};
            clearRect.rect.extent    = {depth_shadow_image->get_width(), depth_shadow_image->get_height()};
            clearRect.baseArrayLayer = 0; // 核心：精确指定清空第 i 层
            clearRect.layerCount     = 1;

            vkCmdClearAttachments(command_buffer_, 1, &clearAttachment, 1, &clearRect);

            uint temp = 3 - i;
            PushConstants(shader_data_ref->pipeline_layout,
                          VK_SHADER_STAGE_VERTEX_BIT, 0, 4, &temp);
            // 现在绑定的管线是有问题的,
            default_status();
            PipelineRasterizationState temp_state;
            temp_state.cullMode_ = VK_CULL_MODE_FRONT_BIT;
            temp_state.write_commands(command_buffer_);

            DrawIndexedIndirect(entity, command_calculate.command_size,
                                command_calculate.light_write_buffer[temp]);
        }
    }
    end_rendering();
}

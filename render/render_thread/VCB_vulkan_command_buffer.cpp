//
// Created by 潘鑫 on 2026/8/16.
//
#include "VCB_vulkan_command_buffer.h"

#include "shader_component.h"

void VCB::reset_current_command_buffer(const uint64_t time_line, VkCommandBuffer command_buffer) {
    command_buffer_ = command_buffer;
    time_line_      = time_line;
    VK_CHECK_RESULT_NOT_EXIT(vkResetCommandBuffer(command_buffer_, 0));

    VkCommandBufferBeginInfo cbBI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK_RESULT_NOT_EXIT(vkBeginCommandBuffer(command_buffer_, &cbBI)); // 所有 vkCmd 都必须在它 之后
    if (query_pool_ != VK_NULL_HANDLE) {
        vkCmdResetQueryPool(command_buffer_, query_pool_, 0, 2);
        vkCmdWriteTimestamp(command_buffer_,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // 执行到哪个阶段时记录
                            query_pool_,
                            0 // query 索引
                           );
    }
    gpu_log_label_info("开始记录时间");
}

void VCB::end_rendering() {
    vkCmdEndRendering(command_buffer_); // 这里和之后的 没有限制
}

void VCB::submit_render_queue(Engine &engine) {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”

    // uint32_t wait_semaphore_len = submit_task->wait_semaphore == VK_NULL_HANDLE ? 0 : 1;
    uint32_t signal_semaphore_len    = 2;
    VkSemaphore signal_semaphores[2] = {
        engine.get_timeline_semaphore(),
        engine.get_can_render_to_image_semaphores()[engine.get_imageIndex()]
    };
    uint64_t signal_semaphore_values[2] = {time_line_, 0};

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        signal_semaphore_len,
        signal_semaphore_values
    };

    Command_submit_manager::command_buffer_submit(1, &command_buffer_,
                                                  engine.get_current_fences(),
                                                  &timeline_semaphore_submit_info,
                                                  1,
                                                  &engine.get_current_presentSemaphores(),
                                                  &waitStages,
                                                  2,
                                                  signal_semaphores);
}

void VCB::compute_write_finish_barrier(const VKR_image_ptr &compute_write_finish_image) {
    VkImageMemoryBarrier2 barrierDrawImage{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        // 之前是在 COMPUTE 阶段进行的写入
        .srcStageMask  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        // 下一步是要在 TRANSFER (拷贝) 阶段作为数据源进行读取
        .dstStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
        // 布局从 Compute 的 GENERAL 切换到最适合拷贝的 TRANSFER_SRC_OPTIMAL
        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        // ⚠️ 填入你自己的专属中转图 Image 句柄
        .image = compute_write_finish_image->get_image_handle(),
        .subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0, .levelCount = 1,
            .baseArrayLayer = 0, .layerCount = 1
        }
    };
    VkDependencyInfo drawImageDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierDrawImage
    };
    vkCmdPipelineBarrier2(command_buffer_, &drawImageDependencyInfo);
}


void VCB::compute_write_finish_same_read(const VKR_image_ptr &compute_write_finish_image) {
    VkImageMemoryBarrier2 barrierDrawImage{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        // 之前是在 COMPUTE 阶段进行的写入
        .srcStageMask  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        .oldLayout     = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout     = VK_IMAGE_LAYOUT_GENERAL,
        // ⚠️ 填入你自己的专属中转图 Image 句柄
        .image = compute_write_finish_image->get_image_handle(),
        .subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0, .levelCount = 1,
            .baseArrayLayer = 0, .layerCount = 1
        }
    };
    VkDependencyInfo drawImageDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierDrawImage
    };
    vkCmdPipelineBarrier2(command_buffer_, &drawImageDependencyInfo);
}

void VCB::compute_write_finish_sample_read(const VKR_image_ptr &compute_write_finish_image) {
    VkImageMemoryBarrier2 barrierDrawImage{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        // 之前是在 COMPUTE 阶段进行的写入
        .srcStageMask  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        // 下一步是要在 TRANSFER (拷贝) 阶段作为数据源进行读取
        .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, // 下一阶段：后处理片元着色器
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,             // 允许着色器读取
        // 布局从 Compute 的 GENERAL 切换到最适合拷贝的 TRANSFER_SRC_OPTIMAL
        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        // ⚠️ 填入你自己的专属中转图 Image 句柄
        .image = compute_write_finish_image->get_image_handle(),
        .subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0, .levelCount = 1,
            .baseArrayLayer = 0, .layerCount = 1
        }
    };
    VkDependencyInfo drawImageDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierDrawImage
    };
    vkCmdPipelineBarrier2(command_buffer_, &drawImageDependencyInfo);
}

void VCB::dof_blur(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths dof_blur{
        "", "", "", "dof_blur"
    };
    auto compute_shader                              = engine.get_shader_manager().find(dof_blur);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, parameter.object_descriptor_sets);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    bind_Proxy_descriptor_sets(parameter.object_descriptor_sets, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::SSAO(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths SSAO{
        "", "", "", "SSAO"
    };
    auto compute_shader                              = engine.get_shader_manager().find(SSAO);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_depth_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::blur_SSAO(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths blur{
        "", "", "", "blur"
    };
    auto compute_shader                              = engine.get_shader_manager().find(blur);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_compute_image2D_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::fxaa(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths fxaa{
        "", "", "", "fxaa"
    };
    auto compute_shader                              = engine.get_shader_manager().find(fxaa);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::dof_composite(Engine &engine,
                        VKR_image_ptr dof_image,
                        VKR_image_ptr color_image,
                        VKR_image_ptr compute_write_image) {
    VKR_shader_paths dof_Chromatic_Aberration_tone_mapping{
        "", "", "", "dof_composite"
    };
    auto compute_shader = engine.get_shader_manager().find(dof_Chromatic_Aberration_tone_mapping);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(compute_write_image);
    std::optional<Texture_parameter> offscreen = create_2d_texture(color_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, parameter.object_descriptor_sets);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    bind_Proxy_descriptor_sets(parameter.object_descriptor_sets, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}


void VCB::compute_write_init_barrier(const VKR_image_ptr &compute_write_finish_image) {
    VkImageMemoryBarrier2 barrierDrawImage{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask  = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout     = VK_IMAGE_LAYOUT_GENERAL,
        .image         = compute_write_finish_image->get_image_handle(),
        .subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };
    VkDependencyInfo drawImageDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierDrawImage
    };
    vkCmdPipelineBarrier2(command_buffer_, &drawImageDependencyInfo);
}

void VCB::end_command_buffer() {
    if (query_pool_ != VK_NULL_HANDLE) {
        vkCmdWriteTimestamp(command_buffer_,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // 执行到哪个阶段时记录
                            query_pool_,
                            1 // query 索引
                           );
    }
    gpu_log_label_info("结束记录时间");

    VkImageMemoryBarrier2 barrierPresent{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = 0,
        .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image         = Engine::instance().get_current_swap_chain_image()->get_image_handle(),
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
    };
    VkDependencyInfo barrierPresentDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierPresent
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierPresentDependencyInfo);
    VK_CHECK_RESULT_NOT_EXIT(vkEndCommandBuffer(command_buffer_)); // 所有 vkCmd 都必须在它 之前
}

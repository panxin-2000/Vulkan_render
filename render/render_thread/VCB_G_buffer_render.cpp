//
// Created by 潘鑫 on 2026/8/13.
//


#include "../engine.h"
#include "VCB_vulkan_command_buffer.h"


G_buffer_image_index VCB::begin_g_buffer_rendering_attachment(
    const std::vector<VKR_image_ptr> &color,
    const VKR_image_ptr &depth, VkAttachmentLoadOp depth_loadOp) {
    // 不存储位置，但是我之前都在存储位置， 之后看看如果更改为这个样子 现在的是 pos normal base_color depth
    // G-Buffer A: 法线 (Normal) + 粗糙度 (Roughness)
    // G-Buffer B: 金属度 (Metallic) + 高光 (Spec) + 遮蔽 (AO)
    // G-Buffer C: 基础色 (BaseColor)
    // Depth Buffer: 深度值（关键就在这里）


    // 这个时候再去申请吗？
    std::vector<VkImageMemoryBarrier2> outputBarriers;
    for (auto image_ptr: color) {
        outputBarriers.push_back(
                                 VkImageMemoryBarrier2{
                                     .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                     .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     .srcAccessMask = 0,
                                     .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     .dstAccessMask =
                                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
                                     .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,

                                     .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                     .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                     .image               = image_ptr->get_image_handle(),
                                     .subresourceRange{
                                         .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                         .baseMipLevel   = 0,
                                         .levelCount     = 1,
                                         .baseArrayLayer = 0,
                                         .layerCount     = 1
                                     }
                                 });
    }
    outputBarriers.push_back(

                             VkImageMemoryBarrier2{
                                 .sType        = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                 .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                 VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                 .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                 .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                 VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                 .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                 .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
                                 .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,

                                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                 .image               = depth->get_image_handle(time_line_),
                                 .subresourceRange{
                                     .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                     .levelCount = 1,
                                     .layerCount = 1
                                 }
                             });

    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);

    std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos;

    for (auto image_ptr: color) {
        colorAttachmentInfos.push_back(VkRenderingAttachmentInfo{
                                           .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                           .imageView   = image_ptr->get_image_view(),
                                           .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                                           .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                                           .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
                                       });
    }
    const VkExtent2D temp_extent = {depth->get_width(), depth->get_height()};
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = depth_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {0.0f, 0}}
    };
    VkRenderingAttachmentInfo StencilAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = depth_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {0.0f, 0}}
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size()),
        .pColorAttachments    = colorAttachmentInfos.data(),
        .pDepthAttachment     = &depthAttachmentInfo,
        .pStencilAttachment   = &StencilAttachmentInfo,
    };
    vkCmdBeginRendering(command_buffer_, &renderingInfo);
    set_pass_viewport(depth->get_width(), depth->get_height());
    set_pass_scissor(depth->get_width(), depth->get_height());

    return {};
}


void VCB::current_write_next_read_image(const std::vector<VKR_image_ptr> &images) {
    std::vector<VkImageMemoryBarrier2> outputBarriers;
    outputBarriers.reserve(images.size());

    for (const auto &image: images) {
        auto temp = init_image_memory_barrier(image,
                                              image_barrier_frag_write_color,
                                              image_barrier_frag_read_sampler2d
                                             );
        outputBarriers.push_back(temp);
    }
    const VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);
}

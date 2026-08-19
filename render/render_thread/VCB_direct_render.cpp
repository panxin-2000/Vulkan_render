//
// Created by 潘鑫 on 2026/8/13.
//


#include "../engine.h"
#include "../render_common/render_state.h"
#include "VCB_vulkan_command_buffer.h"

void VCB::begin_rendering_depth_attachment(VKR_image_ptr depth,
                                           VkAttachmentLoadOp depth_loadOp) {
    std::vector<VkImageMemoryBarrier2> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .image         = depth->get_image_handle(time_line_),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        },
    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);

    auto temp_extent = VK_backend::instance().get_current_extent();
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = depth_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };
    // VkRenderingAttachmentInfo StencilAttachmentInfo{
    //     .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    //     .imageView   = depth->get_image_view(),
    //     .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    //     .loadOp      = depth_loadOp,
    //     .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    //     .clearValue  = {.depthStencil = {1.0f, 0}}
    // };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = 0,
        .pColorAttachments    = nullptr,
        .pDepthAttachment     = &depthAttachmentInfo, // pDepthAttachment 在缩放时有问题。
        .pStencilAttachment   = nullptr
    };
    vkCmdBeginRendering(command_buffer_, &renderingInfo);
}


void VCB::begin_rendering_attachment(VKR_image_ptr color, VKR_image_ptr depth,
                                     VkAttachmentLoadOp depth_loadOp) {
    std::vector<VkImageMemoryBarrier2> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image         = color->get_image_handle(time_line_),
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        },
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .image         = depth->get_image_handle(time_line_),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1, .layerCount = 1
            }
        },
    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);

    VkRenderingAttachmentInfo colorAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = color->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
    };
    auto temp_extent = VK_backend::instance().get_current_extent();
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = depth_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };
    VkRenderingAttachmentInfo StencilAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = depth_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &colorAttachmentInfo,
        .pDepthAttachment     = &depthAttachmentInfo, // pDepthAttachment 在缩放时有问题。
        .pStencilAttachment   = &StencilAttachmentInfo
    };
    vkCmdBeginRendering(command_buffer_, &renderingInfo);
}

void VCB::add_one_indirect_draw_barrier(VkBuffer buffer, VkDeviceSize size,
                                        VkDeviceSize offset) {
    std::array<VkBufferMemoryBarrier2, 1> write_finish_buffer{
        VkBufferMemoryBarrier2{
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext               = nullptr,
            .srcStageMask        = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            .srcAccessMask       = VK_ACCESS_SHADER_WRITE_BIT,
            .dstStageMask        = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
            .dstAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer              = buffer,
            .offset              = offset,
            .size                = size,
        },

    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = 0, // 默认填零，需要VR 或其他选项时才需要填
        .memoryBarrierCount       = 0,
        .pMemoryBarriers          = nullptr,
        .bufferMemoryBarrierCount = write_finish_buffer.size(),
        .pBufferMemoryBarriers    = write_finish_buffer.data(),
        .imageMemoryBarrierCount  = 0,
        .pImageMemoryBarriers     = nullptr,
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);
}

//
// Created by 潘鑫 on 2026/8/13.
//


#include "../engine.h"
#include "../render_common/render_state.h"
#include "VCB_vulkan_command_buffer.h"

void VCB::begin_rendering_depth_attachment(VKR_image_ptr depth,
                                           VkAttachmentLoadOp depth_loadOp) {
    std::vector<VkImageMemoryBarrier2> outputBarriers;
    auto temp = init_image_memory_barrier(depth,
                                          image_barrier_blank_stage,
                                          image_barrier_depth_read_write,
                                          0, depth->get_mipLevels(),
                                          0, depth->get_arrayLayers()
                                         );
    outputBarriers.push_back(temp);


    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);

    VkExtent2D current_depth_extent = {depth->get_width(), depth->get_height()};
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = depth_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {.depthStencil = {0.0f, 0}}
    };

    // 多个附件的长宽必须相等
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .offset = {0, 0},
            .extent = current_depth_extent,
        },
        .layerCount           = depth->get_arrayLayers(),
        .colorAttachmentCount = 0,
        .pColorAttachments    = nullptr,
        .pDepthAttachment     = &depthAttachmentInfo, // pDepthAttachment 在缩放时有问题。
        .pStencilAttachment   = nullptr
    };
    vkCmdBeginRendering(command_buffer_, &renderingInfo);
    set_pass_viewport(depth->get_width(), depth->get_height());
    set_pass_scissor(depth->get_width(), depth->get_height());
}


void VCB::begin_rendering_attachment_to_screen(VKR_image_ptr color, VkAttachmentLoadOp color_loadOp) {
    std::vector<VkImageMemoryBarrier2> outputBarriers;

    auto temp = init_image_memory_barrier(color,
                                          image_barrier_transfer_write_dsr,
                                          image_barrier_frag_write_color
                                         );
    outputBarriers.push_back(temp);

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
        .loadOp      = color_loadOp,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
    };
    const VkExtent2D temp_extent = {color->get_width(), color->get_height()};

    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &colorAttachmentInfo,
        .pDepthAttachment     = nullptr, // pDepthAttachment 在缩放时有问题。
        .pStencilAttachment   = nullptr
    };
    vkCmdBeginRendering(command_buffer_, &renderingInfo);
    // 最后绘制的时候不需要 z-buffer
    set_pass_viewport(color->get_width(), color->get_height());
    set_pass_scissor(color->get_width(), color->get_height());
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

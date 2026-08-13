//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_DIRECT_RENDER_H
#define HELLO_MAC_DIRECT_RENDER_H


#include "../engine.h"
#include "../render_common/render_state.h"


#include "VCB_debug_tag.h"


inline void begin_rendering_attachment(VK_backend &handle, const uint64_t time_line) {
    auto cb = Engine::instance().get_current_command_buffer();

    std::vector<VkImageMemoryBarrier2> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image         = Engine::instance().get_current_swap_chain_image(),
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        },
    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);

    VkRenderingAttachmentInfo colorAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = Engine::instance().get_current_swap_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
    };
    auto temp_extent = VK_backend::instance().get_current_extent();
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = Engine::instance().get_current_depth_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };
    VkRenderingAttachmentInfo StencilAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = Engine::instance().get_current_depth_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
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
    vkCmdBeginRendering(cb, &renderingInfo);
}

inline void add_one_indirect_draw_barrier(VK_backend &handle, VkBuffer buffer, VkDeviceSize size,
                                          VkDeviceSize offset = 0) {
    auto cb = Engine::instance().get_current_command_buffer();
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
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
}



#endif //HELLO_MAC_DIRECT_RENDER_H

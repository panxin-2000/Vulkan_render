//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_SHADOW_RENDER_H
#define HELLO_MAC_SHADOW_RENDER_H

#include "../engine.h"
#include "../render_common/render_state.h"


#include "VCB_debug_tag.h"

inline void begin_shadow_pass(VK_backend &handle, const uint64_t time_line) {
    auto cb          = Engine::instance().get_current_command_buffer();
    auto temp_extent = VK_backend::instance().get_current_extent();

    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = Engine::instance().get_current_depth_view(), //  todo: 这里需要变更
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
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
        .colorAttachmentCount = 0,
        .pColorAttachments    = nullptr,
        .pDepthAttachment     = &depthAttachmentInfo,
        .pStencilAttachment   = &StencilAttachmentInfo

    };
    vkCmdBeginRendering(cb, &renderingInfo);
}


inline void shadow_pass_barrier(VK_backend &handle, const uint64_t time_line) {
    auto cb = Engine::instance().get_current_command_buffer();
    std::vector<VkImageMemoryBarrier2> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, // 确保写入缓存刷新
            .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,        // 下一阶段：后处理片元着色器
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,                    // 允许着色器读取
            .oldLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,             // 渲染时布局
            .newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,       // 读取时布局

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = Engine::instance().get_current_depth_image(), // todo: 这里也需要更改
            .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        },
    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<uint32_t>(outputBarriers.size()),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
}


#endif //HELLO_MAC_SHADOW_RENDER_H

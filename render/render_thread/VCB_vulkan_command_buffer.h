//
// Created by 潘鑫 on 2026/1/28.
//

#ifndef HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#define HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#include "../engine.h"
#include "VCB_debug_tag.h"

inline void reset_current_command_buffer(VK_backend &handle, VkQueryPool queryPool, const uint64_t time_line) {
    auto cb = Engine::instance().get_current_command_buffer();
    VK_CHECK_RESULT_NOT_EXIT(vkResetCommandBuffer(cb, 0));

    VkCommandBufferBeginInfo cbBI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK_RESULT_NOT_EXIT(vkBeginCommandBuffer(cb, &cbBI)); // 所有 vkCmd 都必须在它 之后
    if (queryPool != VK_NULL_HANDLE) {
        vkCmdResetQueryPool(cb, queryPool, 0, 2);
        vkCmdWriteTimestamp(cb,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // 执行到哪个阶段时记录
                            queryPool,
                            0 // query 索引
                           );
    }
    gpu_log_label_info(cb, "开始记录时间");
}


inline void end_rendering(VK_backend &engine) {
    auto cb = Engine::instance().get_current_command_buffer();
    vkCmdEndRendering(cb); // 这里和之后的 没有限制
}

inline void end_command_buffer(VK_backend &engine, VkQueryPool queryPool, const uint64_t time_line) {
    auto cb = Engine::instance().get_current_command_buffer();
    if (queryPool != VK_NULL_HANDLE) {
        vkCmdWriteTimestamp(cb,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // 执行到哪个阶段时记录
                            queryPool,
                            1 // query 索引
                           );
    }
    gpu_log_label_info(cb, "结束记录时间");

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
        .sType                = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrierPresent
    };
    vkCmdPipelineBarrier2(cb, &barrierPresentDependencyInfo);
    VK_CHECK_RESULT_NOT_EXIT(vkEndCommandBuffer(cb)); // 所有 vkCmd 都必须在它 之前
}


#endif //HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H

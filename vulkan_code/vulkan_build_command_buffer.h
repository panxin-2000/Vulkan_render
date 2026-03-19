//
// Created by 潘鑫 on 2026/1/28.
//

#ifndef HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#define HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#include "descriptor.h"
#include "engine.h"
#include "render_proxy.h"
#include "vertex_and_buffer_index.h"

struct scoped_debug_label {
    VkCommandBuffer cmd;

    scoped_debug_label(const VkCommandBuffer &cb, const std::string &label) : cmd(cb) {
        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = label.c_str();
        labelInfo.color[0]   = 1.0f; // R (0.0~1.0)
        labelInfo.color[1]   = 1.0f; // G
        labelInfo.color[2]   = 0.0f; // B (黄色)
        labelInfo.color[3]   = 1.0f; // A
        vkCmdBeginDebugUtilsLabelEXT(cmd, &labelInfo);
    };

    ~scoped_debug_label() {
        vkCmdEndDebugUtilsLabelEXT(cmd);
    };
};

/**
 * @brief 为 Vulkan 对象设置调试名称
 * @param backend
 * @param objectType 对象类型 (如 VK_OBJECT_TYPE_BUFFER)
 * @param handle 对象句柄 (强转为 uint64_t)
 * @param name
 */
inline void SetDebugName(const VK_backend &backend,
                         const VkObjectType objectType,
                         const uint64_t handle,
                         const std::string &name) {
    if (vkSetDebugUtilsObjectNameEXT && !name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType                         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType                    = objectType;
        nameInfo.objectHandle                  = handle;
        nameInfo.pObjectName                   = name.c_str();

        vkSetDebugUtilsObjectNameEXT(backend.get_device(), &nameInfo);
    }
}


inline void gpu_log_label_info(const VkCommandBuffer &cb, const std::string &label) {
    // 添加一个函数 ， 颜色根据不同的类型来确定
    VkDebugUtilsLabelEXT markerInfo{};
    markerInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    markerInfo.pLabelName = label.c_str();
    markerInfo.color[0]   = 1.0f; // R (0.0~1.0)
    markerInfo.color[1]   = 1.0f; // G
    markerInfo.color[2]   = 0.0f; // B (黄色)
    markerInfo.color[3]   = 1.0f; // A
    vkCmdInsertDebugUtilsLabelEXT(cb, &markerInfo);
}

inline void reset_current_command_buffer(VK_backend &handle, VkQueryPool queryPool, const uint64_t time_line) {
    auto cb = handle.engine_.get_current_command_buffer();
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

inline void begin_rendering_attachment(VK_backend &handle, const uint64_t time_line) {
    auto cb = handle.engine_.get_current_command_buffer();

    std::array<VkImageMemoryBarrier2, 2> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image         = VK_backend::get().get_current_swap_chain_image(),
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}
        },
        VkImageMemoryBarrier2{
            .sType        = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .image         = VK_backend::get().get_current_depth_image(),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1,
                .layerCount = 1
            }
        }
    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 2,
        .pImageMemoryBarriers = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);

    VkRenderingAttachmentInfo colorAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = handle.get_current_swap_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
    };
    auto temp_extent = VK_backend::get().get_current_extent();
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = VK_backend::get().get_current_depth_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
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
        .pDepthAttachment     = &depthAttachmentInfo
    };
    vkCmdBeginRendering(cb, &renderingInfo);
}

inline void begin_g_buffer_rendering_attachment(VK_backend &handle, const uint64_t time_line) {
    auto cb = handle.engine_.get_current_command_buffer();
    std::array<VkImageMemoryBarrier2, 4> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_position_image(),
            .subresourceRange{
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        },

        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_normal_image(),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        },
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED, // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_baseColor_image(),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        },
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,          // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, // 也行VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_depth_image(),
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        }
    };
    VkDependencyInfo barrierDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = outputBarriers.size(),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);

    std::array<VkRenderingAttachmentInfo, 3> colorAttachmentInfos{
        VkRenderingAttachmentInfo{
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = handle.get_current_position_view(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        },
        VkRenderingAttachmentInfo{
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = handle.get_current_normal_view(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        },
        VkRenderingAttachmentInfo{
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = handle.get_current_baseColor_view(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        },
    };
    auto temp_extent = VK_backend::get().get_current_extent();
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = VK_backend::get().get_current_depth_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = colorAttachmentInfos.size(),
        .pColorAttachments    = colorAttachmentInfos.data(),
        .pDepthAttachment     = &depthAttachmentInfo
    };
    vkCmdBeginRendering(cb, &renderingInfo);
}

inline void g_buffer_attachment_barrier(VK_backend &handle, const uint64_t time_line) {
    auto cb = handle.engine_.get_current_command_buffer();
    std::array<VkImageMemoryBarrier2, 4> outputBarriers{
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // 等待颜色输出完成
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,          // 确保写入缓存刷新
            .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,         // 下一阶段：后处理片元着色器
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,                     // 允许着色器读取
            .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,        // 渲染时布局
            .newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,        // 读取时布局

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_position_image(),
            .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        },

        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // 等待颜色输出完成
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,          // 确保写入缓存刷新
            .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,         // 下一阶段：后处理片元着色器
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,                     // 允许着色器读取
            .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,        // 渲染时布局
            .newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,        // 读取时布局

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_normal_image(),
            .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        },
        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // 等待颜色输出完成
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,          // 确保写入缓存刷新
            .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,         // 下一阶段：后处理片元着色器
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,                     // 允许着色器读取
            .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,        // 渲染时布局
            .newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,        // 读取时布局

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_baseColor_image(),
            .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        },

        VkImageMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,  // 确保写入缓存刷新
            .dstStageMask  = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,         // 下一阶段：后处理片元着色器
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,                     // 允许着色器读取
            .oldLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,              // 渲染时布局
            .newLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, // 读取时布局

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = VK_backend::get().get_current_depth_image(),
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
        .imageMemoryBarrierCount = outputBarriers.size(),
        .pImageMemoryBarriers    = outputBarriers.data()
    };
    vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
}


inline void build_command_buffer(VK_backend &engine, VKR_object_proxy &vk_draw, const uint64_t time_line) {
    const auto cb = engine.engine_.get_current_command_buffer();


    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.vk_pipeline);

    vkCmdSetViewport(cb, 0, 1, &vk_draw.viewport);
    vkCmdSetScissor(cb, 0, 1, &vk_draw.scissor);
    vkCmdSetDepthTestEnable(cb, VK_TRUE);
    vkCmdSetDepthWriteEnable(cb, VK_TRUE);
    vkCmdSetDepthCompareOp(cb, VK_COMPARE_OP_LESS_OR_EQUAL);
    vkCmdSetDepthBoundsTestEnable(cb, VK_FALSE);
    vkCmdSetStencilTestEnable(cb, VK_TRUE);
    // vkCmdSetDepthBiasEnable
    vkCmdSetStencilOp(
                      cb,
                      VK_STENCIL_FACE_FRONT_AND_BACK, // 作用范围
                      VK_STENCIL_OP_KEEP,             // failOp
                      VK_STENCIL_OP_REPLACE,          // passOp
                      VK_STENCIL_OP_KEEP,             // depthFailOp
                      VK_COMPARE_OP_ALWAYS            // compareOp
                     );
    vkCmdSetDepthBounds(cb, 0.5f, 0.8f);

    vkCmdSetDepthBiasEnable(cb, VK_TRUE);
    // 如果开启了，通常紧接着需要设置具体的偏移数值
    vkCmdSetDepthBias(cb,
                      1.25f, // constantFactor (固定偏移)
                      0.0f,  // clamp (最大偏移限制)
                      1.75f  // slopeFactor (随坡度增加的偏移)
                     );

    // VkPipelineDepthStencilStateCreateFlags    flags;
    // VkBool32                                  depthTestEnable;
    // VkBool32                                  depthWriteEnable;
    // VkCompareOp                               depthCompareOp;
    // VkBool32                                  depthBoundsTestEnable;
    // VkBool32                                  stencilTestEnable;
    // VkStencilOpState                          front;
    // VkStencilOpState                          back;
    // float                                     minDepthBounds;
    // float                                     maxDepthBounds;

    if (!vk_draw.vk_descriptor_sets.empty()) {
        std::vector<VkDescriptorSet> temp_descriptor_sets;
        temp_descriptor_sets.resize(vk_draw.vk_descriptor_sets.size());
        for (size_t i = 0; i < vk_draw.vk_descriptor_sets.size(); ++i) {
            temp_descriptor_sets[i] = vk_draw.vk_descriptor_sets[i]->get_descriptor_set(time_line);
        }
        for (auto temp_descriptor_set: temp_descriptor_sets) {
            if (temp_descriptor_set == VK_NULL_HANDLE) {
                LOG_INFO(g_log(), "VKR_object_proxy {} descriptor_set == VK_NULL_HANDLE ", vk_draw.debug_name);
                return;
            }
        }
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vk_draw.pipeline_layout,
                                0,
                                temp_descriptor_sets.size(),
                                temp_descriptor_sets.data(), 0,
                                nullptr);
    }
    // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 允许不绑定部分描述符，只要不犯法就是允许的
    // 访问的时候不在也是可以的，不会出现明显的死机，只是内容没有绘制

    // if (vk_draw.push_constants_pool != nullptr) {
    //     auto push_constants_address = vk_draw.push_constants_pool->get_gpu_device_address(time_line);
    //     vkCmdPushConstants(cb, vk_draw.pipeline_layout,
    //                        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint64_t),
    //                        &push_constants_address);
    // }
    for (int i = 0; i < vk_draw.mesh.size(); ++i) {
        vk_draw.mesh[i].draw(cb, time_line);
    }
}

inline void build_deferred_command_buffer(VK_backend &engine, VKR_object_proxy &vk_draw, const uint64_t time_line) {
    const auto cb = engine.engine_.get_current_command_buffer();

    vkCmdSetViewport(cb, 0, 1, &vk_draw.viewport);
    vkCmdSetScissor(cb, 0, 1, &vk_draw.scissor);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.vk_pipeline);
    if (!vk_draw.vk_descriptor_sets.empty()) {
        std::vector<VkDescriptorSet> temp_descriptor_sets;
        temp_descriptor_sets.resize(vk_draw.vk_descriptor_sets.size());
        for (size_t i = 0; i < vk_draw.vk_descriptor_sets.size(); ++i) {
            temp_descriptor_sets[i] = vk_draw.vk_descriptor_sets[i]->get_descriptor_set(time_line);
        }
        for (auto temp_descriptor_set: temp_descriptor_sets) {
            if (temp_descriptor_set == VK_NULL_HANDLE) {
                LOG_INFO(g_log(), "VKR_object_proxy {} descriptor_set == VK_NULL_HANDLE ", vk_draw.debug_name);
                return;
            }
        }
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vk_draw.pipeline_layout,
                                0,
                                temp_descriptor_sets.size(),
                                temp_descriptor_sets.data(), 0,
                                nullptr);
    }
    // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 允许不绑定部分描述符，只要不犯法就是允许的
    // 访问的时候不在也是可以的，不会出现明显的死机，只是内容没有绘制

    // if (vk_draw.push_constants_pool != nullptr) {
    //     auto push_constants_address = vk_draw.push_constants_pool->get_gpu_device_address(time_line);
    //     vkCmdPushConstants(cb, vk_draw.pipeline_layout,
    //                        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint64_t),
    //                        &push_constants_address);
    // }
    vkCmdDraw(cb, 3, 1, 0, 0);
}

inline void end_rendering(VK_backend &engine) {
    auto cb = engine.engine_.get_current_command_buffer();
    vkCmdEndRendering(cb); // 这里和之后的 没有限制
}

inline void end_command_buffer(VK_backend &engine, VkQueryPool queryPool, const uint64_t time_line) {
    auto cb = engine.engine_.get_current_command_buffer();
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
        .image         = engine.get_current_swap_chain_image(),
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

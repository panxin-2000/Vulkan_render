//
// Created by 潘鑫 on 2026/1/28.
//

#ifndef HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#define HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#include "../vulkan_code/descriptor.h"
#include "../engine.h"
#include "render_proxy.h"
#include "../vulkan_code/vertex_and_buffer_index.h"
#include "name_component.h"

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
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &colorAttachmentInfo,
        .pDepthAttachment     = &depthAttachmentInfo // pDepthAttachment 在缩放时有问题。
    };
    vkCmdBeginRendering(cb, &renderingInfo);
}


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
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{
            .extent = temp_extent,
        },
        .layerCount           = 1,
        .colorAttachmentCount = 0,
        .pColorAttachments    = nullptr,
        .pDepthAttachment     = &depthAttachmentInfo
    };
    vkCmdBeginRendering(cb, &renderingInfo);
}


struct G_buffer_image_index {
    uint32_t position_image_index;
    uint32_t normal_image_index;
    uint32_t baseColor_image_index;
};

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

inline G_buffer_image_index begin_g_buffer_rendering_attachment(VK_backend &handle, const uint64_t time_line) {
    // 不存储位置，但是我之前都在存储位置， 之后看看如果更改为这个样子 现在的是 pos normal base_color depth
    // G-Buffer A: 法线 (Normal) + 粗糙度 (Roughness)
    // G-Buffer B: 金属度 (Metallic) + 高光 (Spec) + 遮蔽 (AO)
    // G-Buffer C: 基础色 (BaseColor)
    // Depth Buffer: 深度值（关键就在这里）


    auto cb = Engine::instance().get_current_command_buffer();
    // 这个时候再去申请吗？
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
            .image               = Engine::instance().get_current_position_image(),
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
            .image               = Engine::instance().get_current_normal_image(),
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
            .image               = Engine::instance().get_current_baseColor_image(),
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
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,          // 不关心旧布局的内容，丢弃
            .newLayout     = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, // 也行VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = Engine::instance().get_current_depth_image(),
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
            .imageView   = Engine::instance().get_current_position_view(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        },
        VkRenderingAttachmentInfo{
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = Engine::instance().get_current_normal_view(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        },
        VkRenderingAttachmentInfo{
            .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView   = Engine::instance().get_current_baseColor_view(),
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
            .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}}
        },
    };
    auto temp_extent = VK_backend::instance().get_current_extent();
    VkRenderingAttachmentInfo depthAttachmentInfo{
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = Engine::instance().get_current_depth_view(),
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
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
    return {};
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


inline void g_buffer_attachment_barrier(VK_backend &handle, const uint64_t time_line) {
    auto cb = Engine::instance().get_current_command_buffer();
    std::vector<VkImageMemoryBarrier2> outputBarriers{
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
            .image               = Engine::instance().get_current_position_image(),
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
            .image               = Engine::instance().get_current_normal_image(),
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
            .image               = Engine::instance().get_current_baseColor_image(),
            .subresourceRange    = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
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

inline void bind_Proxy_descriptor_sets(VK_backend &engine, entt::entity entity, const uint64_t time_line,
                                       VkPipelineBindPoint bind_point) {
    const auto cb                 = Engine::instance().get_current_command_buffer();
    const auto vk_descriptor_sets = update_descriptor_sets(entity);
    if (!vk_descriptor_sets.empty()) {
        std::vector<VkDescriptorSet> temp_descriptor_sets;
        temp_descriptor_sets.resize(vk_descriptor_sets.size());
        for (size_t i = 0; i < vk_descriptor_sets.size(); ++i) {
            temp_descriptor_sets[i] = vk_descriptor_sets[i]->get_descriptor_set(time_line);
            // LOG_INFO(g_log(), "temp_descriptor_sets[{}] = {}", i, (uint64_t)temp_descriptor_sets[i]);
        }
        std::vector<uint32_t> dynamic_offsets; // dynamic
        // dynamic_offsets.resize(vk_descriptor_sets.size());
        // for (size_t i = 0; i < vk_descriptor_sets.size(); ++i) {
        // dynamic_offsets[i] = 0;
        // }
        for (auto temp_descriptor_set: temp_descriptor_sets) {
            if (temp_descriptor_set == VK_NULL_HANDLE) {
                LOG_INFO(g_log(), "VKR_object_proxy {} descriptor_set == VK_NULL_HANDLE ",
                         Render_entt().get<Name_component>(entity).name_);
                return;
            }
        }
        vkCmdBindDescriptorSets(cb, bind_point,
                                Render_entt().get<VkPipelineLayout>(entity),
                                0,
                                temp_descriptor_sets.size(),
                                temp_descriptor_sets.data(),
                                dynamic_offsets.size(),
                                dynamic_offsets.data());
    }
}


inline void build_compute_dispatch(VK_backend &engine, entt::entity entity, const uint64_t time_line) {
    const auto cb = Engine::instance().get_current_command_buffer();
    // 下面一行估计还是有问题
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, Render_entt().get<VkPipeline>(entity));
    bind_Proxy_descriptor_sets(engine, entity, time_line, VK_PIPELINE_BIND_POINT_COMPUTE);

    if (const auto group_count = Render_entt().try_get<compute_group_count>(entity)) {
        vkCmdDispatch(cb, group_count->X, group_count->Y, group_count->Z);
    }
    // layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in; // 工作组大小
    // 上面是什么内容呢？ 第一个需要理解的是，wave,  local_size 的 总数必须是 wave (32或64) 的整数倍
    // local_size 总是是分配在一个 CU 中
    // 单个工作组（Workgroup / Group）中的所有线程，绝对会被分配在同一个硬件计算单元中执行，无法跨单元拆分
    //    共享内存 (shared / Local Data Share, LDS)：同一个 Group 内的线程可以通过高速片上缓存直接交换数据。
    //    组内同步 (barrier() / 屏障)：可以强制让组内所有线程暂停，直到大家都走到这一步。
    // 上面的两个特征限制了 单个 CU 中的数量
    // 如果需要更多的线程数量，那么就需要在 group_count 中申请更多的数量了
    //    layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
    //    vkCmdDispatch(cmd, 32, 32, 1);
    //    512 * 512 分辨率的 图片，gl_GlobalInvocationID.x 对应行坐标，gl_GlobalInvocationID.y 对应列坐标
    //  只能保证 16 * 16 个 的一个 局部图片在一个 CU 中
    //  虽然都是可以凑到需要的线程数，但是需要考虑怎么凑才能 访问 绝对连续的显存空间，触发全速合并访问
    // }
}

inline void build_command_buffer(VK_backend &engine, entt::entity entity, const uint64_t time_line) {
    const auto cb = Engine::instance().get_current_command_buffer();

    auto debug_name = Render_entt().get<Name_component>(entity).name_;
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, Render_entt().get<VkPipeline>(entity));

    vkCmdSetDepthTestEnable(cb, VK_TRUE);
    vkCmdSetDepthCompareOp(cb, VK_COMPARE_OP_LESS_OR_EQUAL);

    //
    vkCmdSetDepthWriteEnable(cb, VK_TRUE);

    vkCmdSetDepthBoundsTestEnable(cb, VK_FALSE);
    vkCmdSetStencilTestEnable(cb, VK_FALSE);

    vkCmdSetDepthBoundsTestEnable(cb, VK_FALSE);
    vkCmdSetDepthBiasEnable(cb, VK_FALSE);


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

    bind_Proxy_descriptor_sets(engine, entity, time_line, VK_PIPELINE_BIND_POINT_GRAPHICS);

    // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 允许不绑定部分描述符，只要不犯法就是允许的
    // 访问的时候不在也是可以的，不会出现明显的死机，只是内容没有绘制

    //  shader_data 还没有传送过来
    const auto &shader_data_ref = Render_entt().get<shader_data>(entity);
    if (const auto parameter = Render_entt().try_get<shader_need_parameter>(entity))
        for (auto &[name,value]: shader_data_ref->push_constant_map) {
            vkCmdPushConstants(cb, Render_entt().get<VkPipelineLayout>(entity),
                               value.stageFlags, value.offset, value.size,
                               parameter->push_constant_pool + value.offset);
        }

    const auto mesh_data  = Render_entt().get<Mesh_data>(entity);
    const auto primitives = Render_entt().get<std::vector<VKR_Primitive> >(entity);
    if (!primitives.empty()) {
        for (auto &primitive: primitives) {
            vkCmdSetViewport(cb, 0, 1, &primitive.viewport);
            vkCmdSetScissor(cb, 0, 1, &primitive.scissor);
            primitive.draw(cb, mesh_data, time_line);
        }
    } else {
        // 为空并且有一个deferred 标记 // todo: 标记判断
        if (Render_entt().any_of<deferred_pass_tag>(entity))
            vkCmdDraw(cb, 3, 1, 0, 0);
        if (Render_entt().any_of<UI_2D_tag>(entity))
            vkCmdDraw(cb, 4, 1, 0, 0);
    }
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
        .image         = Engine::instance().get_current_swap_chain_image(),
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

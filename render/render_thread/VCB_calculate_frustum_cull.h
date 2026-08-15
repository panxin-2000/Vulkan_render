//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_CALCULATE_FRUSTUM_CULL_H
#define HELLO_MAC_CALCULATE_FRUSTUM_CULL_H
#include "../render_component/GPU_frustum_cull.h"
#include "engine.h"
#include "global_singleton.h"

inline void calculate_frustum_cull(const VkCommandBuffer &cb,
                                   const entt::entity entity,
                                   const FrustumPlanes &frustum_planes,
                                   const uint64_t timeline) {
    {
        auto command_shader = Engine::instance().get_frustum_cull_shader_data();
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);

        auto command_calculate           = Render_entt().get<GPU_frustum_cull>(entity);
        command_calculate.frustum_planes = frustum_planes; // 还需要在这里更新一次
        vkCmdPushConstants(cb, command_shader->pipeline_layout,
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
                           &command_calculate);
        vkCmdDispatch(cb, ALIGN_256(command_calculate.command_size) / 256, 1, 1);

        auto &parameter               = Render_entt().get_or_emplace<shader_need_parameter>(entity);
        VKR_buffer_ptr command_buffer = command_calculate.command_buffer;

        std::array<VkBufferMemoryBarrier2, 1> write_buffer{
            VkBufferMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, // 1. 修正 stype 类型
                .pNext = nullptr,
                // 2. 优化 Stage：前一个阶段是 Compute Shader 执行完成
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                // 3. 优化 Access：前一个动作是 Compute Shader 的写入完成
                .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,

                // 4. 关键：接下来的阶段是 间接绘制命令读取（Draw Indirect Fetch）
                .dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
                // 5. 关键：接下来的动作是 读取间接参数缓冲区（Indirect Buffer Read）
                .dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,

                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

                // 6. 填入你那个存放 Indirect Commands 的实际 VkBuffer 句柄
                .buffer = command_buffer->get_buffer_handle(timeline), // 这里的buffer 句柄 应该从哪里拿?
                .offset = 0,
                // 7. 填入该缓冲区的实际字节大小，或使用 VK_WHOLE_SIZE 覆盖整块内存
                .size = VK_WHOLE_SIZE,
            }
        };
        VkDependencyInfo barrierDependencyInfo{
            .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext                    = nullptr,
            .dependencyFlags          = 0, // 默认填零，需要VR 或其他选项时才需要填
            .memoryBarrierCount       = 0,
            .pMemoryBarriers          = nullptr,
            .bufferMemoryBarrierCount = write_buffer.size(),
            .pBufferMemoryBarriers    = write_buffer.data(),
            .imageMemoryBarrierCount  = 0,
            .pImageMemoryBarriers     = nullptr,
        };
        vkCmdPipelineBarrier2(cb, &barrierDependencyInfo);
    }
}

#endif //HELLO_MAC_CALCULATE_FRUSTUM_CULL_H

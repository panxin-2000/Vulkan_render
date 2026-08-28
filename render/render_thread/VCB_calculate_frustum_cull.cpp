//
// Created by 潘鑫 on 2026/8/13.
//

#include "../render_component/GPU_frustum_cull.h"
#include "engine.h"
#include "global_singleton.h"
#include "object_ply.h"
#include "VCB_vulkan_command_buffer.h"


// 想办法,不断的把这六个函数填写完整就可以了,应该就能过运行了

void VCB::render_3DGS_preprocess(const entt::entity entity) {
    if (Render_entt().all_of<object_3DGS_parameters>(entity)) {
        VKR_shader_paths temp{
            "", "", "", "3DGS/preprocess"
        };
        auto command_push_const = Render_entt().get<object_3DGS_parameters>(entity);

        auto command_shader = Engine::instance().get_shader_manager().find(temp);

        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);

        bind_Proxy_descriptor_sets(entity,
                                   command_shader->pipeline_layout,
                                   VK_PIPELINE_BIND_POINT_COMPUTE);

        vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
                           &command_push_const);
        vkCmdDispatch(command_buffer_, ALIGN_256(command_push_const.gaussianCount) / 256, 1, 1);
    }
}

void VCB::render_3DGS_prefixsum(const entt::entity entity) {
    // VKR_shader_paths temp{
    //     "", "", "", "3DGS/prefixsum"
    // };
    // auto command_shader = Engine::instance().get_shader_manager().find(temp);
    // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
    //                    &command_calculate);
    // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
}

void VCB::render_3DGS_idkeys(const entt::entity entity) {
    // VKR_shader_paths temp{
    //     "", "", "", "3DGS/idkeys"
    // };
    // auto command_shader = Engine::instance().get_shader_manager().find(temp);
    // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
    //                    &command_calculate);
    // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
}

void VCB::render_3DGS_histogram_radixsort(const entt::entity entity) { {
        // VKR_shader_paths temp{
        //     "", "", "", "3DGS/histogram"
        // };
        // auto command_shader = Engine::instance().get_shader_manager().find(temp);
        // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
        //
        // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
        //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
        //                    &command_calculate);
        // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
    } {
        // VKR_shader_paths temp{
        //     "", "", "", "3DGS/radixsort"
        // };
        // auto command_shader = Engine::instance().get_shader_manager().find(temp);
        // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
        //
        // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
        //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
        //                    &command_calculate);
        // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
    }
}

void VCB::render_3DGS_tile_boundaries(const entt::entity entity) {
    // VKR_shader_paths temp{
    //     "", "", "", "3DGS/tile_boundaries"
    // };
    // auto command_shader = Engine::instance().get_shader_manager().find(temp);
    // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
    //                    &command_calculate);
    // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
}

void VCB::render_3DGS_render(const entt::entity entity) {
    // VKR_shader_paths temp{
    //     "", "", "", "3DGS/render"
    // };
    // auto command_shader = Engine::instance().get_shader_manager().find(temp);
    // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
    //                    &command_calculate);
    // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
}


void VCB::calculate_frustum_cull(const entt::entity entity, const FrustumPlanes &frustum_planes,
                                 const std::array<FrustumPlanes, 4> &light_frustum_planes) {
    {
        auto command_shader = Engine::instance().get_shader_manager().get_frustum_cull_shader_data();
        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
        std::vector<VkBufferMemoryBarrier2> write_buffer_barriers;

        auto command_calculate                    = Render_entt().get<GPU_frustum_cull>(entity);
        command_calculate.frustum_planes          = frustum_planes; // 还需要在这里更新一次
        command_calculate.IndirectCommandsAddress = command_calculate.camera_write_buffer->get_gpu_device_address();
        vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
                           &command_calculate);
        vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
        VKR_buffer_ptr write_buffer_ptr = command_calculate.camera_write_buffer;
        write_buffer_barriers.push_back(VkBufferMemoryBarrier2{
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
                                            .buffer = write_buffer_ptr->get_buffer_handle(time_line_),
                                            // 这里的buffer 句柄 应该从哪里拿?
                                            .offset = 0,
                                            // 7. 填入该缓冲区的实际字节大小，或使用 VK_WHOLE_SIZE 覆盖整块内存
                                            .size = VK_WHOLE_SIZE,
                                        });


        for (uint32_t i = 0;
             i < light_frustum_planes.size() && command_calculate.light_write_buffer[i] != nullptr; ++i) {
            command_calculate.frustum_planes          = light_frustum_planes[i]; // 还需要在这里更新一次
            command_calculate.IndirectCommandsAddress =
                    command_calculate.light_write_buffer[i]->get_gpu_device_address();
            vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
                               VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
                               &command_calculate);
            vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
            write_buffer_barriers.push_back(VkBufferMemoryBarrier2{
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
                                                .buffer = command_calculate.light_write_buffer[i]->
                                                get_buffer_handle(time_line_),
                                                // 这里的buffer 句柄 应该从哪里拿?
                                                .offset = 0,
                                                // 7. 填入该缓冲区的实际字节大小，或使用 VK_WHOLE_SIZE 覆盖整块内存
                                                .size = VK_WHOLE_SIZE,
                                            });
        }
        auto &parameter = Render_entt().get_or_emplace<shader_need_parameter>(entity);


        VkDependencyInfo barrierDependencyInfo{
            .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext                    = nullptr,
            .dependencyFlags          = 0, // 默认填零，需要VR 或其他选项时才需要填
            .memoryBarrierCount       = 0,
            .pMemoryBarriers          = nullptr,
            .bufferMemoryBarrierCount = static_cast<uint32_t>(write_buffer_barriers.size()),
            .pBufferMemoryBarriers    = write_buffer_barriers.data(),
            .imageMemoryBarrierCount  = 0,
            .pImageMemoryBarriers     = nullptr,
        };
        vkCmdPipelineBarrier2(command_buffer_, &barrierDependencyInfo);
    }
}

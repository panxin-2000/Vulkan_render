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
        add_barriers({command_push_const.tilesTouched_ptr});
    }
}

VKR_buffer_ptr VCB::render_3DGS_prefixsum(const entt::entity entity) {
    VKR_shader_paths temp{
        "", "", "", "3DGS/prefixsum"
    };
    auto command_shader     = Engine::instance().get_shader_manager().find(temp);
    auto command_push_const = Render_entt().get<object_3DGS_parameters>(entity);

    uint32_t prefixSumGroups = ALIGN_256(command_push_const.gaussianCount) / 256;
    uint32_t _numSteps       = static_cast<uint32_t>(std::ceil(std::log2(command_push_const.gaussianCount)));
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);

    VKR_buffer_ptr result;
    for (uint32_t step = 0; step <= _numSteps; step++) {
        // 每次 一半，一半的一半，一直相加
        struct PushConstants {
            uint64_t buffer_A_Address; //  command_push_const.tilesTouched_ptr
            uint64_t buffer_B_Address; //  和上面大小相同的一个 buffer
            uint32_t step;
            int32_t numElements;
            int32_t readFromA;
        } pushConstants = {
            command_push_const.tilesTouched_ptr->get_gpu_device_address(),
            command_push_const.tilesTouched_Prefix_Sum_ptr->get_gpu_device_address(),
            step,
            int32_t(command_push_const.gaussianCount),
            (step % 2) == 0 ? 1 : 0,
        };
        // 还是需要确定最后的 输出的结果是那个 buffer 上的内容
        if (pushConstants.readFromA == 1) {
            result = command_push_const.tilesTouched_Prefix_Sum_ptr;
        } else if (pushConstants.readFromA == 0) {
            result = command_push_const.tilesTouched_ptr;
        }
        vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants),
                           &pushConstants);
        // 这里确实是将全部的组都运行了一遍
        vkCmdDispatch(command_buffer_, prefixSumGroups, 1, 1);

        add_barriers({result});
    }
    return result;
}

void VCB::render_3DGS_idkeys(const entt::entity entity, const VKR_buffer_ptr &prefix_sum) {
    VKR_shader_paths temp{
        "", "", "", "3DGS/idkeys"
    };
    auto command_shader = Engine::instance().get_shader_manager().find(temp);

    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    auto command_push_const = Render_entt().get<object_3DGS_parameters>(entity);

    struct PushConstants {
        uint64_t TilesSum_Address;
        uint64_t Depths_Address;
        uint64_t BoundingBox_Address;
        uint64_t Out_keysUnsorted_Address;
        uint64_t Out_valuesUnsorted_Address;
        uint tileX;
        int nGauss;
    };

    PushConstants pushconstants;
    pushconstants.nGauss                     = command_push_const.gaussianCount;
    pushconstants.TilesSum_Address           = prefix_sum->get_gpu_device_address(); // 需要从上一轮中 读取出来
    pushconstants.Depths_Address             = command_push_const.depth_address;
    pushconstants.BoundingBox_Address        = command_push_const.bbox_address;
    pushconstants.Out_keysUnsorted_Address   = command_push_const.keysUnsorted->get_gpu_device_address();
    pushconstants.Out_valuesUnsorted_Address = command_push_const.valuesUnsorted->get_gpu_device_address();

    vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
                       VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants),
                       &pushconstants);
    vkCmdDispatch(command_buffer_, ALIGN_256(command_push_const.gaussianCount) / 256, 1, 1);
}

/**
 *
 * @param entity
 * @param keys
 * @param histograms histograms 直方图
 * @param keysRadix
 * @param values
 * @param valuesRadix
 */
void VCB::render_3DGS_histogram_radixsort(const entt::entity entity,
                                          VKR_buffer_ptr keys,
                                          VKR_buffer_ptr histograms,
                                          VKR_buffer_ptr keysRadix,
                                          VKR_buffer_ptr values,
                                          VKR_buffer_ptr valuesRadix) {
    struct RadixHistogramPushConstants {
        uint32_t g_num_elements;
        uint32_t g_shift;
        uint32_t g_num_workgroups;
        uint32_t g_num_blocks_per_workgroup;

        uint64_t g_elements_in_address;
        uint64_t g_histograms_address;
    } radixPC;

    struct RadixSortPushConstants {
        uint32_t g_num_elements;
        uint32_t g_shift;
        uint32_t g_num_workgroups;
        uint32_t g_num_blocks_per_workgroup;
        uint64_t g_elements_in_address;
        uint64_t g_elements_out_address;
        uint64_t g_payload_in_address;
        uint64_t g_payload_out_address;
        uint64_t g_histograms_address;
    } radix_sort_PC;

    uint32_t numElementsToSort    = 50000000;
    uint32_t blocks_per_workgroup = 32;
    uint32_t elementsPerWorkgroup = 256 * blocks_per_workgroup;
    // WORKGROUP_SIZE * blocks_per_workgroup; // 256 * 32 = 8192
    uint32_t numWorkgroups =
            (numElementsToSort + elementsPerWorkgroup - 1) / elementsPerWorkgroup;


    radixPC.g_num_elements             = 50000000;
    radixPC.g_num_workgroups           = numWorkgroups;
    radixPC.g_num_blocks_per_workgroup = blocks_per_workgroup;

    // Perform 6 passes of radix sort (tiles_ID always can be represented with 2 bits)

    for (uint32_t pass = 0; pass < 6; pass++) {
        radixPC.g_shift = pass * 8;


        bool isEven = (pass % 2 == 0); {
            VKR_shader_paths temp{
                "", "", "", "3DGS/histogram"
            };
            auto command_shader = Engine::instance().get_shader_manager().find(temp);
            vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
            if (isEven == false) {
                radixPC.g_elements_in_address = keys->get_gpu_device_address();       // keys
                radixPC.g_histograms_address  = histograms->get_gpu_device_address(); // histograms
            } else {
                radixPC.g_elements_in_address = keysRadix->get_gpu_device_address();  // keysRadix
                radixPC.g_histograms_address  = histograms->get_gpu_device_address(); // histograms
            }
            //
            // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
            //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
            //                    &command_calculate);
            // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
        } {
            VKR_shader_paths temp{
                "", "", "", "3DGS/radixsort"
            };


            if (isEven == false) {
                radix_sort_PC.g_elements_in_address  = keys->get_gpu_device_address();        // keys
                radix_sort_PC.g_elements_out_address = keysRadix->get_gpu_device_address();   // keysRadix
                radix_sort_PC.g_payload_in_address   = values->get_gpu_device_address();      // values
                radix_sort_PC.g_payload_out_address  = valuesRadix->get_gpu_device_address(); // valuesRadix
                radix_sort_PC.g_histograms_address   = histograms->get_gpu_device_address();  // histograms
            } else {
                radix_sort_PC.g_elements_in_address  = keysRadix->get_gpu_device_address();   // keysRadix
                radix_sort_PC.g_elements_out_address = keys->get_gpu_device_address();        // keys
                radix_sort_PC.g_payload_in_address   = valuesRadix->get_gpu_device_address(); // valuesRadix
                radix_sort_PC.g_payload_out_address  = values->get_gpu_device_address();      // values
                radix_sort_PC.g_histograms_address   = histograms->get_gpu_device_address();  // histograms
            }


            auto command_shader = Engine::instance().get_shader_manager().find(temp);
            vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
            //
            // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
            //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
            //                    &command_calculate);
            // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
        }
    }
}

/**
 * 找出每个 Tile 负责的高斯点的起始索引和结束索引
 * @param entity
 */
void VCB::render_3DGS_tile_boundaries(const entt::entity entity) {
    VKR_shader_paths temp{
        "", "", "", "3DGS/tile_boundaries"
    };
    auto command_shader = Engine::instance().get_shader_manager().find(temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
    //                    &command_calculate);
    // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
}


void VCB::render_3DGS_render(const entt::entity entity) {
    VKR_shader_paths temp{
        "", "", "", "3DGS/render"
    };
    auto command_shader = Engine::instance().get_shader_manager().find(temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);
    //
    // vkCmdPushConstants(command_buffer_, command_shader->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
    //                    &command_calculate);
    // vkCmdDispatch(command_buffer_, ALIGN_256(command_calculate.command_size) / 256, 1, 1);
}

void VCB::copy_image(VKR_image_ptr src_image, VKR_image_ptr dst_image) {
    VkImageMemoryBarrier2 barrierDrawImage{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask  = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask  = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image         = dst_image->get_image_handle(),
        .subresourceRange{
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1
        }
    };
    VkDependencyInfo drawImageDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierDrawImage
    };
    vkCmdPipelineBarrier2(command_buffer_, &drawImageDependencyInfo);


    if (src_image->get_width() == dst_image->get_width() && src_image->get_height() == dst_image->get_height()) {
        VkImageCopy copyRegion{
            .srcSubresource = 0,
            .srcOffset      = {0, 0, 0},
            .dstSubresource = 0,
            .dstOffset      = {0, 0, 0},
            .extent         = {1280, 720, 1}
        };
        vkCmdCopyImage(command_buffer_, src_image->get_image_handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dst_image->get_image_handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &copyRegion);
    } else {
        VkImageBlit blitRegion{};
        // 源范围：你的中转图大小 (0,0) 到 (Width, Height)
        blitRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.mipLevel       = 0;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount     = 1;
        blitRegion.srcOffsets[0]                 = {0, 0, 0};
        blitRegion.srcOffsets[1]                 = {
            static_cast<int32_t>(src_image->get_width()),
            static_cast<int32_t>(src_image->get_height()),
            1
        };

        // 目标范围：当前交换链的大小 (0,0) 到 (SwapchainWidth, SwapchainHeight)
        blitRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.mipLevel       = 0;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount     = 1;

        blitRegion.dstOffsets[0] = {0, 0, 0};
        blitRegion.dstOffsets[1] = {
            static_cast<int32_t>(dst_image->get_width()),
            static_cast<int32_t>(dst_image->get_height()),
            1
        };

        // 🚀 一发 Blit，带上 LINEAR 过滤，大小不一致也能完美适配
        vkCmdBlitImage(command_buffer_,
                       src_image->get_image_handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dst_image->get_image_handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blitRegion, VK_FILTER_LINEAR);
    }
}

void VCB::deal_image(const entt::entity entity, const VKR_image_ptr &write) {
    VKR_shader_paths temp{
        "", "", "", "draw_circle"
    };

    const auto &shader_data_ref = Render_entt().get<Shader_data>(entity);
    bind_pipeline_update_parameter(entt::null, shader_data_ref);


    auto width   = write->get_width();
    auto height  = write->get_height();
    float radius = 100;

    // vkCmdPushConstants(command_buffer_, shader_data_ref->pipeline_layout,
    //                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 4,
    //                    &radius);


    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}


void VCB::add_barriers(const std::vector<VKR_buffer_ptr> &buffer_ptrs) const {
    std::vector<VkBufferMemoryBarrier2> write_buffer_barriers;
    write_buffer_barriers.reserve(buffer_ptrs.size());
    for (const auto &buffer_ptr: buffer_ptrs) {
        write_buffer_barriers.push_back(VkBufferMemoryBarrier2{
                                            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, // 1. 修正 stype 类型
                                            .pNext = nullptr,
                                            .srcStageMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                                            .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
                                            .dstStageMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                                            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                                            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                            .buffer = buffer_ptr->get_buffer_handle(time_line_),
                                            .offset = 0,
                                            .size = VK_WHOLE_SIZE,
                                        });
    }
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

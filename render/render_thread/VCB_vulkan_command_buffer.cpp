//
// Created by 潘鑫 on 2026/8/16.
//
#include "VCB_vulkan_command_buffer.h"

#include "shader_component.h"

void VCB::reset_current_command_buffer(const uint64_t time_line, VkCommandBuffer command_buffer) {
    command_buffer_ = command_buffer;
    time_line_      = time_line;
    VK_CHECK_RESULT_NOT_EXIT(vkResetCommandBuffer(command_buffer_, 0));

    VkCommandBufferBeginInfo cbBI{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK_RESULT_NOT_EXIT(vkBeginCommandBuffer(command_buffer_, &cbBI)); // 所有 vkCmd 都必须在它 之后
    if (query_pool_ != VK_NULL_HANDLE) {
        vkCmdResetQueryPool(command_buffer_, query_pool_, 0, 2);
        vkCmdWriteTimestamp(command_buffer_,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // 执行到哪个阶段时记录
                            query_pool_,
                            0 // query 索引
                           );
    }
    gpu_log_label_info("开始记录时间");
}

void VCB::end_rendering() {
    vkCmdEndRendering(command_buffer_); // 这里和之后的 没有限制
}

void VCB::submit_render_queue(Engine &engine) {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”

    // uint32_t wait_semaphore_len = submit_task->wait_semaphore == VK_NULL_HANDLE ? 0 : 1;
    uint32_t signal_semaphore_len    = 2;
    VkSemaphore signal_semaphores[2] = {
        engine.get_timeline_semaphore(),
        engine.get_can_render_to_image_semaphores()[engine.get_imageIndex()]
    };
    uint64_t signal_semaphore_values[2] = {time_line_, 0};

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        signal_semaphore_len,
        signal_semaphore_values
    };

    Command_submit_manager::command_buffer_submit(1, &command_buffer_,
                                                  engine.get_current_fences(),
                                                  &timeline_semaphore_submit_info,
                                                  1,
                                                  &engine.get_current_presentSemaphores(),
                                                  &waitStages,
                                                  2,
                                                  signal_semaphores);
}

void VCB::compute_write_finish_barrier(const VKR_image_ptr &compute_write_finish_image) {
    add_image_barrier(compute_write_finish_image, image_barrier_compute_write_image2D, image_barrier_transfer_read_src);
}


void VCB::compute_write_finish_same_read(const VKR_image_ptr &compute_write_finish_image) {
    add_image_barrier(compute_write_finish_image, image_barrier_compute_write_image2D,
                      image_barrier_compute_read_sampler2D);
}

void VCB::compute_write_finish_sample_read(const VKR_image_ptr &compute_write_finish_image) {
    add_image_barrier(compute_write_finish_image, image_barrier_compute_write_image2D,
                      image_barrier_fragment_read_sampler2d);
}

void VCB::dof_blur(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths dof_blur{
        "", "", "", "dof_blur"
    };
    auto compute_shader                              = engine.get_shader_manager().find(dof_blur);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, parameter.object_descriptor_sets);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    bind_Proxy_descriptor_sets(parameter.object_descriptor_sets, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::SSAO(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths SSAO{
        "", "", "", "SSAO"
    };
    auto compute_shader                              = engine.get_shader_manager().find(SSAO);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_depth_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::blur_SSAO(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths blur{
        "", "", "", "blur"
    };
    auto compute_shader                              = engine.get_shader_manager().find(blur);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_compute_image2D_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = out_image->get_width();
    auto height = out_image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::only_image_compute(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image,
                             const std::string &compute_path) {
    VKR_shader_paths fxaa{
        "", "", "", compute_path
    };
    auto compute_shader                              = engine.get_shader_manager().find(fxaa);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = out_image->get_width();
    auto height = out_image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}


void VCB::CAS(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths fxaa{
        "", "", "", "CAS_shader"
    };
    auto compute_shader                              = engine.get_shader_manager().find(fxaa);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_compute_image2D_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "imgSrc",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "imgDst",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = out_image->get_width();
    auto height = out_image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_8(height) / 8, 1);
}


#define A_CPU
#include "./../shader/ffx_a.h"
#include "./../shader/ffx_fsr1.h"


void LpmSetupOut(AU1 i,inAU4 v, AU1 *address) {
    address[i * 4 + 0] = v[0];
    address[i * 4 + 1] = v[1];
    address[i * 4 + 2] = v[2];
    address[i * 4 + 3] = v[3];
}

#include "./../shader/ffx_lpm.h"


class LpmConfigGenerator {
private:
    // 核心：保存当前外部传入的局部内存地址指针
    AU1 ctl[24 * 4];

public:
    void *init() {
        varAF3(saturation) = initAF3(0.0, 0.0, 0.0);
        varAF3(crosstalk)  = initAF3(1.0, 1.0/2.0, 1.0/32.0);
        LpmSetup(
                 false, LPM_CONFIG_709_709, LPM_COLORS_709_709, // <-- Using the LPM_ prefabs to make inputs easier.
                 0.0,                                           // softGap
                 256.0,                                         // hdrMax
                 8.0,                                           // exposure
                 0.25,                                          // contrast
                 1.0,                                           // shoulder contrast
                 saturation, crosstalk, ctl);
        return ctl;
    }
};


struct FSRConstants {
    Eigen::Vector4f Const0;
    Eigen::Vector4f Const1;
    Eigen::Vector4f Const2;
    Eigen::Vector4f Const3;
};


void VCB::FSR1_EASU(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths easu{
        "", "", "", "fsr_1_pass"
    };
    easu.add_define_macro("SAMPLE_EASU", 1);
    easu.add_define_macro("SAMPLE_RCAS", 0);
    auto compute_shader = engine.get_shader_manager().find(easu);

    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    // 下面一行不对, 还需要
    std::optional<Texture_parameter> offscreen = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "InputTexture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "OutputTexture",
                         compute_texture);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "InputSampler",
                         offscreen);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = out_image->get_width();
    auto height = out_image->get_height();


    FSRConstants consts = {};
    FsrEasuCon(reinterpret_cast<AU1 *>(&consts.Const0),
               reinterpret_cast<AU1 *>(&consts.Const1),
               reinterpret_cast<AU1 *>(&consts.Const2),
               reinterpret_cast<AU1 *>(&consts.Const3),
               static_cast<AF1>(input_image->get_width()),
               static_cast<AF1>(input_image->get_height()),
               static_cast<AF1>(input_image->get_width()),
               static_cast<AF1>(input_image->get_height()),
               static_cast<AF1>(out_image->get_width()),
               static_cast<AF1>(out_image->get_height()));

    PushConstants(compute_shader->pipeline_layout,
                  VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FSRConstants), &consts);


    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

void VCB::FSR1_RCAS(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths rcas{
        "", "", "", "fsr_1_pass"
    };
    rcas.add_define_macro("SAMPLE_EASU", 0);
    rcas.add_define_macro("SAMPLE_RCAS", 1);
    auto compute_shader                              = engine.get_shader_manager().find(rcas);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "InputTexture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "OutputTexture",
                         compute_texture);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "InputSampler",
                         offscreen);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = out_image->get_width();
    auto height = out_image->get_height();

    FSRConstants consts   = {};
    float rcasAttenuation = 0.25f;
    FsrRcasCon(reinterpret_cast<AU1 *>(&consts.Const0), rcasAttenuation);

    PushConstants(compute_shader->pipeline_layout,
                  VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FSRConstants), &consts);

    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}

struct tone_mapping_Constants {
    Eigen::Vector4f Const0[24];
};


void VCB::down_sample(Engine &engine, const VKR_image_ptr image_ptr, const std::string &compute_path) {
    const Image_and_view_parameters &parameters = image_ptr->get_parameters();
    int32_t mipWidth                            = parameters.width;
    int32_t mipHeight                           = parameters.height;
    VKR_shader_paths down_sample{
        "", "", "", compute_path
    };
    auto compute_shader = engine.get_shader_manager().find(down_sample);

    // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);

    for (uint32_t i = 1; i < parameters.mipLevels; i++) {
        // 初始化将要写入的每一层
        {
            add_image_barrier(image_ptr,
                              image_barrier_blank_stage,
                              image_barrier_compute_write_image2D,
                              i);
        }
        // 执行每一层的计算
        // {
        //     std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(image_ptr);
        //     std::optional<Texture_parameter> offscreen       = create_2d_texture(image_ptr);
        //
        //     // 逻辑还是看起来都差不多 , 但是最好能再上面的时候添加一个 总的汇总
        //     shader_need_parameter parameter;
        //     set_render_parameter(compute_shader->object_sets_bindings,
        //                          parameter.update_object_descriptor_sets, "InputTexture",
        //                          offscreen);
        //     set_render_parameter(compute_shader->object_sets_bindings,
        //                          parameter.update_object_descriptor_sets, "OutputTexture",
        //                          compute_texture);
        //     allocate_descriptor_sets(parameter, compute_shader);
        //     auto temp = get_descriptor_sets(parameter, compute_shader);
        //     update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
        //     bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
        //                                VK_PIPELINE_BIND_POINT_COMPUTE);
        //
        //     auto width  = mipWidth > 1 ? mipWidth / 2 : 1;
        //     auto height = mipHeight > 1 ? mipHeight / 2 : 1;
        //
        //     // 还需要创建多个 view , 之后再上传,之后 还需要清理掉
        //     vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
        //     if (mipWidth > 1) mipWidth /= 2;
        //     if (mipHeight > 1) mipHeight /= 2;
        // }
        // 执行计算完成之后的转换
        {
            add_image_barrier(image_ptr,
                              image_barrier_compute_write_image2D,
                              image_barrier_compute_read_sampler2D,
                              i);
        }
    }
}

void VCB::up_sample(Engine &engine, const VKR_image_ptr image_ptr, const std::string &compute_path) {
    const Image_and_view_parameters &parameters = image_ptr->get_parameters();
    int32_t mipWidth                            = parameters.width;
    int32_t mipHeight                           = parameters.height;
    VKR_shader_paths down_sample{
        "", "", "", compute_path
    };
    auto compute_shader = engine.get_shader_manager().find(down_sample);

    // vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);

    for (uint32_t i = 1; i < parameters.mipLevels; i++) {
        // 初始化将要写入的每一层
        {
            add_image_barrier(image_ptr,
                              image_barrier_blank_stage,
                              image_barrier_compute_write_image2D,
                              i);
        }
        // 执行每一层的计算
        // {
        //     std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(image_ptr);
        //     std::optional<Texture_parameter> offscreen       = create_2d_texture(image_ptr);
        //
        //     // 逻辑还是看起来都差不多 , 但是最好能再上面的时候添加一个 总的汇总
        //     shader_need_parameter parameter;
        //     set_render_parameter(compute_shader->object_sets_bindings,
        //                          parameter.update_object_descriptor_sets, "InputTexture",
        //                          offscreen);
        //     set_render_parameter(compute_shader->object_sets_bindings,
        //                          parameter.update_object_descriptor_sets, "OutputTexture",
        //                          compute_texture);
        //     allocate_descriptor_sets(parameter, compute_shader);
        //     auto temp = get_descriptor_sets(parameter, compute_shader);
        //     update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
        //     bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
        //                                VK_PIPELINE_BIND_POINT_COMPUTE);
        //
        //     auto width  = mipWidth > 1 ? mipWidth / 2 : 1;
        //     auto height = mipHeight > 1 ? mipHeight / 2 : 1;
        //
        //     // 还需要创建多个 view , 之后再上传,之后 还需要清理掉
        //     vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
        //     if (mipWidth > 1) mipWidth /= 2;
        //     if (mipHeight > 1) mipHeight /= 2;
        // }
        // 执行计算完成之后的转换
        {
            add_image_barrier(image_ptr,
                              image_barrier_compute_write_image2D,
                              image_barrier_compute_read_sampler2D,
                              i);
        }
    }
}

void VCB::tone_mapping(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image) {
    VKR_shader_paths tone_mapping{
        "", "", "", "tone_mapping"
    };
    auto compute_shader                              = engine.get_shader_manager().find(tone_mapping);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(out_image);
    std::optional<Texture_parameter> offscreen       = create_2d_texture(input_image);

    LpmConfigGenerator config_gen;
    auto address = config_gen.init();


    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "tone_parameters",
                         config_gen);
    allocate_descriptor_sets(parameter, compute_shader);
    auto temp = get_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, temp);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    // parameter.object_descriptor_sets 需要去确认 或者说需要更新
    bind_Proxy_descriptor_sets(temp, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);


    auto width  = out_image->get_width();
    auto height = out_image->get_height();

    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}


void VCB::dof_composite(Engine &engine,
                        VKR_image_ptr dof_image,
                        VKR_image_ptr color_image,
                        VKR_image_ptr compute_write_image) {
    VKR_shader_paths dof_Chromatic_Aberration_tone_mapping{
        "", "", "", "dof_composite"
    };
    auto compute_shader = engine.get_shader_manager().find(dof_Chromatic_Aberration_tone_mapping);
    std::optional<Texture_parameter> compute_texture = create_compute_image2D_texture(compute_write_image);
    std::optional<Texture_parameter> offscreen = create_2d_texture(color_image);

    shader_need_parameter parameter;
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "input_texture",
                         offscreen);
    set_render_parameter(compute_shader->object_sets_bindings,
                         parameter.update_object_descriptor_sets, "out_texture",
                         compute_texture);
    allocate_descriptor_sets(parameter, compute_shader);
    update_descriptor_sets(parameter.update_object_descriptor_sets, parameter.object_descriptor_sets);
    vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, compute_shader->pipeline_t);
    bind_Proxy_descriptor_sets(parameter.object_descriptor_sets, compute_shader->pipeline_layout,
                               VK_PIPELINE_BIND_POINT_COMPUTE);
    auto width  = compute_texture->image->get_width();
    auto height = compute_texture->image->get_height();
    vkCmdDispatch(command_buffer_, ALIGN_16(width) / 16, ALIGN_16(height) / 16, 1);
}


void VCB::add_image_barrier(const VKR_image_ptr &image,
                            const VkPipelineStageFlags srcStageMask,
                            const VkAccessFlags srcAccessMask,
                            const VkImageLayout oldLayout,
                            const VkPipelineStageFlags dstStageMask,
                            const VkAccessFlags dstAccessMask,
                            const VkImageLayout newLayout,
                            const uint32_t baseMipLevel,
                            const uint32_t levelCount,
                            const uint32_t baseArrayLayer,
                            const uint32_t layerCount) {
    VkImageMemoryBarrier2 barrierDrawImage{
        .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask  = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask  = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout     = oldLayout,
        .newLayout     = newLayout,
        .image         = image->get_image_handle(),
        .subresourceRange{
            .aspectMask     = image->get_aspectMask(),
            .baseMipLevel   = baseMipLevel,
            .levelCount     = levelCount,
            .baseArrayLayer = baseArrayLayer,
            .layerCount     = layerCount
        }
    };
    VkDependencyInfo drawImageDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierDrawImage
    };
    vkCmdPipelineBarrier2(command_buffer_, &drawImageDependencyInfo);
}

void VCB::compute_write_init_barrier(const VKR_image_ptr &compute_write_finish_image) {
    add_image_barrier(compute_write_finish_image, image_barrier_blank_stage, image_barrier_compute_write_image2D);
}

void VCB::end_command_buffer() {
    if (query_pool_ != VK_NULL_HANDLE) {
        vkCmdWriteTimestamp(command_buffer_,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // 执行到哪个阶段时记录
                            query_pool_,
                            1 // query 索引
                           );
    }
    gpu_log_label_info("结束记录时间");

    auto barrierPresent = init_image_memory_barrier(Engine::instance().get_current_swap_chain_image(),
                                                    image_barrier_frag_write_color,
                                                    image_barrier_present_src_khr
                                                   );
    VkDependencyInfo barrierPresentDependencyInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers    = &barrierPresent
    };
    vkCmdPipelineBarrier2(command_buffer_, &barrierPresentDependencyInfo);
    VK_CHECK_RESULT_NOT_EXIT(vkEndCommandBuffer(command_buffer_)); // 所有 vkCmd 都必须在它 之前
}

//
// Created by 潘鑫 on 2026/8/16.
//
#include "create_pipeline.h"
#include "shader_create.h"
#include "vertex_and_buffer_index.h"
#include "VCB_vulkan_command_buffer.h"
#include "vk_render_to_image.h"
#include "GPU_frustum_cull.h"
#include "framerate_measure.h"
#include "name_component.h"
#include "object_ply.h"
#include "vulkan_render_manage.h"
#include "time_measure.h"
#include "transform_component.h"
#include "vulkan_execute_command.h"


void render_different_pass(VCB &vcb,
                           Engine &engine,
                           VKR_image_ptr color_image,
                           VKR_image_ptr depth_image,
                           VKR_image_ptr depth_AO_image,
                           VKR_image_ptr depth_AO_copy_image,
                           VKR_image_ptr SSAO_image,
                           VKR_image_ptr blur_SSAO_image,
                           VKR_image_ptr depth_shadow_image,
                           VKR_image_ptr entity_image,
                           VKR_image_ptr fxaa_result,
                           VKR_image_ptr EASU_result,
                           VKR_image_ptr RCAS_result,
                           VKR_image_ptr compute_dof_blur_image
) {
    std::array<VkBufferMemoryBarrier2, 1> write_buffer{
        VkBufferMemoryBarrier2{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext         = nullptr,
            .srcStageMask  = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstStageMask  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,

            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer              = VK_NULL_HANDLE,
            .offset              = 0,
            .size                = 1,
        },

    };
    // dispatch 不能 render pass 中间调用
    {
        // Indirect draw ,先说需要哪几个 buffer ，
        // 需要准备一个需要写入 command 的 buffer
        // 数据，有哪些不透明的物体需要绘制的，位置或包围盒，
        // 然后是对应的不透明的物体 draw command ,只是被复制，不需要被重新计算
        // 上面是最简单的
        // Hi-Z 遮挡剔除 需要什么呢？
        // 一个深度图
        // 上一帧绘制的 command 的 buffer
        //
        auto view = Render_entt().view<compute_pass_tag>();
        for (const auto entity: view) {
            vcb.build_compute_dispatch(entity);
        }
        // add_one_indirect_draw_barrier(handle,VK_NULL_HANDLE, 1024);
    }
    auto frustum_planes = engine.get_frustum_planes();
    auto light_planes   = engine.get_light_frustum_planes();
    // 视锥裁剪
    if (engine.get_frustum_culling() == true) {
        auto view = Render_entt().view<GPU_frustum_cull>();
        for (const auto entity: view)
            vcb.calculate_frustum_cull(entity, frustum_planes, light_planes, depth_AO_copy_image);
    }

    // 应该是在 需要 deferred 的时候才开启
    {
        auto view = Render_entt().view<deferred_pass_tag>();
        if (!view.empty()) {
            auto g_buffer_image_indices = vcb.begin_g_buffer_rendering_attachment(
                 {
                     color_image, engine.get_image_manager().get_one_position_image(),
                     engine.get_image_manager().get_one_normal_image()
                 }, depth_image, VK_ATTACHMENT_LOAD_OP_CLEAR);
            auto view_opacity = Render_entt().view<opacity_tag, Name_component>();
            for (const auto entity: view_opacity) {
                auto name = Render_entt().get<Name_component>(entity);
                vcb.build_draw_command(entity);
            }
            vcb.end_rendering();
            vcb.current_write_next_read_image({
                                                  color_image,
                                                  engine.get_image_manager().get_one_position_image(),
                                                  engine.get_image_manager().get_one_normal_image()
                                              });
        }
    }

    // 这里是绘制 不透明 ,得到深度图, 还是
    // 不能按照
    {
        vcb.begin_g_buffer_rendering_attachment({entity_image}, depth_AO_image,
                                                VK_ATTACHMENT_LOAD_OP_CLEAR);
        auto view = Render_entt().view<opacity_tag, GPU_frustum_cull, Name_component, VKR_shader_paths>();
        for (const auto entity: view) {
            auto command_calculate = Render_entt().get<GPU_frustum_cull>(entity);
            auto name              = Render_entt().get<Name_component>(entity);
            auto shader_path       = Render_entt().get<VKR_shader_paths>(entity);
            shader_path.clear_define_macro();
            shader_path.depthAttachmentFormat_   = VK_FORMAT_D32_SFLOAT;
            shader_path.stencilAttachmentFormat_ = VK_FORMAT_UNDEFINED;
            shader_path.add_define_macro("PASS_DEPTH_AND_PICKUP", 1);
            const auto &shader_data_ref =
                    engine.get_shader_manager().find(shader_path);
            vcb.bind_pipeline_update_parameter(entity, shader_data_ref);
            // 现在绑定的管线是有问题的,
            vcb.default_status();

            vcb.DrawIndexedIndirect(entity, command_calculate.command_size, command_calculate.camera_write_buffer);
        }
        vcb.end_rendering();

        auto &mouse_position = engine.get_global_parameters().get_mouse_position();
        vcb.pickup(entity_image, engine.get_pickup_buffers(), mouse_position.x(), mouse_position.y());

        vcb.add_image_barrier(depth_AO_image,
                              image_barrier_depth_write,
                              image_barrier_compute_read_sampler2D);
        // 这里需要什么呢?  // depth_AO_image 中复制到 depth_AO_copy_image ,
        // 之后 进行降采样以及 上采样
        // 不能直接搬运,可以通过一个shader 来执行转换

        // vcb.compute_write_init_barrier(depth_AO_copy_image);


        vcb.add_image_barrier(depth_AO_copy_image,
                              image_barrier_blank_stage,
                              image_barrier_compute_write_image2D,
                              0,
                              depth_AO_copy_image->get_mipLevels());
        vcb.only_image_compute(engine, depth_AO_image, depth_AO_copy_image, "copy_from_depth_to_AO");
        vcb.add_image_barrier(depth_AO_copy_image,
                              image_barrier_compute_write_image2D,
                              image_barrier_compute_read_sampler2D);


        vcb.down_sample(engine, depth_AO_copy_image, "mipmap_depth");


        vcb.compute_write_init_barrier(SSAO_image);
        vcb.SSAO(engine, depth_AO_copy_image, SSAO_image, engine.get_global_parameters().projection_matrix);
        vcb.compute_write_finish_same_read(SSAO_image);

        vcb.compute_write_init_barrier(blur_SSAO_image);
        vcb.blur_SSAO(engine, SSAO_image, blur_SSAO_image, {2, 0});
        vcb.compute_write_finish_sample_read({blur_SSAO_image});

        vcb.compute_write_init_barrier(SSAO_image);
        vcb.blur_SSAO(engine, blur_SSAO_image, SSAO_image, {0, 2});
        vcb.compute_write_finish_sample_read({SSAO_image});

        // simple_mipmap(vcb.get_command_buffer(), depth_AO_image, depth_AO_image->get_parameters());
        // 之后还需要执行什么操作呢?  进行采样
        // 这里大概需要需要生成 Mipmap
        // 其实搞好能够用于 SSAO
    } {
        //     // 这里还是稍微有点问题,其实是可以不要深度的
        // vcb.begin_rendering_attachment(SSAO_image,
        //                                depth_image,
        //                                VK_ATTACHMENT_LOAD_OP_CLEAR);

        // SSAO_image 需要转换布局,从 开始的 未知 转换为 gen
    }


    // CSM  当然了,这里还是有一个问题, 最好能不需要渲染全部的,
    {
        vcb.CSM_pass(engine, depth_shadow_image);
        vcb.add_image_barrier(depth_shadow_image,
                              image_barrier_depth_write,
                              image_barrier_frag_read_sampler2d);
    }
    // {
    // vcb.begin_rendering_attachment(blur_SSAO_image,
    // depth_image,
    // VK_ATTACHMENT_LOAD_OP_CLEAR);
    // VKR_shader_paths blur{
    // "full_screen_triangle", "blur", "", "",
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    // VK_FORMAT_UNDEFINED,
    // VK_FORMAT_UNDEFINED,
    // };
    // auto command_shader = engine.get_shader_manager().find(blur);
    // vcb.render_post_deal(command_shader, entt::null);
    // vcb.end_rendering();
    // vcb.current_write_next_read_image({
    // blur_SSAO_image
    // });
    // }

    // 绘制 3d 物体的阶段 pass
    {
        {
            auto view = Render_entt().view<deferred_pass_tag>();
            if (!view.empty()) {
                vcb.begin_g_buffer_rendering_attachment({color_image}, depth_image, VK_ATTACHMENT_LOAD_OP_LOAD);
            } else {
                vcb.begin_g_buffer_rendering_attachment({color_image,}, depth_image, VK_ATTACHMENT_LOAD_OP_CLEAR);
            }
        }
        // 应该先划分不同的 pass 阶段，
        //  deferred  不应该将深度值写入的
        {
            // g_buffer_image_indices 这是需要看看怎么传递进入其中
            auto view = Render_entt().view<deferred_pass_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
            }
        } {
            // 按照常理来说，包围盒的时候 深度比较出问题了，所以会覆盖
            auto view = Render_entt().view<std::vector<VKR_Primitive>, skybox_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
            }
        } {
            auto view = Render_entt().view<opacity_tag, GPU_frustum_cull, Name_component>();
            for (const auto entity: view) {
                auto command_calculate      = Render_entt().get<GPU_frustum_cull>(entity);
                auto name                   = Render_entt().get<Name_component>(entity);
                const auto &shader_data_ref = Render_entt().get<Shader_data>(entity);
                vcb.bind_pipeline_update_parameter(entity, shader_data_ref);
                vcb.DrawIndexedIndirect(entity, command_calculate.command_size, command_calculate.camera_write_buffer);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>,
                                           opacity_tag,
                                           Name_component>();
            for (const auto entity: view) {
                auto name = Render_entt().get<Name_component>(entity);
                vcb.build_draw_command(entity);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, translate_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
            }
        } {
            auto view = Render_entt().view<volume_pass_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
            }
        }
        vcb.end_rendering();
        vcb.current_write_next_read_image({
                                              color_image
                                          });
    }
    // 那么这里是否可以插入 其他的内容呢?
    //

    {
        auto view = Render_entt().view<ply_3DGS_tag>();
        for (const auto entity: view) {
            auto command_push_const = Render_entt().get<object_3DGS_parameters>(entity);
            vcb.render_3DGS_preprocess(entity);
            auto prefix_sum = vcb.render_3DGS_prefixsum(entity);
            vcb.render_3DGS_idkeys(entity, prefix_sum);
        }
    } {
        // 去缺陷 → 还原彩色 → 擦除噪点 → 提亮暗部 → 调出好看的颜色 → 最终压缩（如 JPEG）输出

        // 景深（Depth of Field）绽放（Bloom）镜片炫光（Lens Flare）
        // 镜头色差（Chromatic Aberration） —— （在这里引入通道颜色位移）
        // 色调映射与色彩校正（Tone Mapping & Color Grading） —— （将 HDR 转换为 LDR）


        vcb.compute_write_init_barrier(compute_dof_blur_image); // 忘记这里是什么了


        vcb.compute_write_init_barrier(fxaa_result);
        vcb.only_image_compute(engine, color_image, fxaa_result, "fxaa");
        vcb.compute_write_finish_same_read(fxaa_result);

        vcb.compute_write_init_barrier(EASU_result);
        vcb.FSR1_EASU(engine, fxaa_result, EASU_result);
        vcb.compute_write_finish_same_read(EASU_result);

        vcb.compute_write_init_barrier(RCAS_result);
        vcb.FSR1_RCAS(engine, EASU_result, RCAS_result);
        vcb.compute_write_finish_barrier(RCAS_result);

        vcb.copy_image(RCAS_result, engine.get_current_swap_chain_image());
    } {
        vcb.begin_rendering_attachment_to_screen(engine.get_current_swap_chain_image(),
                                                 VK_ATTACHMENT_LOAD_OP_LOAD); {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, UI_2D_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command_UI(entity);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, UI_render_text>();
            for (const auto entity: view) {
                vcb.build_draw_command_UI(entity);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, Line_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command_UI(entity);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, imgui_draw>();
            for (const auto entity: view) {
                vcb.build_draw_command_UI(entity);
            }
        }
        vcb.end_rendering();
    }
}


void destroy_Render_entt() { {
        const auto view = Render_entt().view<Render_destroy_tag_last>();
        Render_entt().destroy(view.begin(), view.end()); // 执行销毁程序
    } {
        const auto view = Render_entt().view<Render_destroy_tag>();
        for (const auto entity: view) {
            Render_entt().remove<Render_destroy_tag>(entity);
            Render_entt().emplace<Render_destroy_tag_last>(entity);
        }
        // 这里的执行销毁是有问题的, 应该是需要 再等一次才能够 删除
        // 最好还是放在 get_image_to_render 之后 才会完全没有问题
    }
}

void vk_render_GPU::render_once(VK_backend &backend, Engine &engine) {
    VK_backend::instance().update_current_extent();
    engine.get_image_manager().using_to_free();
    const auto color_image            = engine.get_image_manager().get_one_color_image();
    const auto entity_image           = engine.get_image_manager().get_one_entity_image();
    const auto depth_image            = engine.get_image_manager().get_one_depth_image();
    const auto depth_AO_image         = engine.get_image_manager().get_one_depth_AO_image();
    const auto depth_AO_copy_image    = engine.get_image_manager().get_one_depth_AO_copy_image();
    const auto SSAO_image             = engine.get_image_manager().get_one_depth_SSAO_image();
    const auto depth_shadow_image     = engine.get_image_manager().get_one_shadow_image();
    const auto fxaa_result            = engine.get_image_manager().get_one_compute_write_image();
    const auto RCAS_result            = engine.get_image_manager().get_one_post_process_finish_image();
    const auto EASU_result            = engine.get_image_manager().get_one_post_process_finish_image();
    const auto compute_dof_blur_image = engine.get_image_manager().get_one_compute_write_image();
    // 之后呢? 怎么绑定呢?

    const auto blur_SSAO_image = engine.get_image_manager().get_one_depth_SSAO_image(); {
        std::unique_lock<std::mutex> lock(mtx);

        auto offscreen      = create_2d_texture(color_image);
        auto depth          = create_2d_texture(depth_AO_image);
        auto blur_SSAO      = create_2d_texture(SSAO_image);
        auto shadow_texture = create_2d_texture(depth_shadow_image);

        engine.update_global_parameter(offscreen,
                                       depth,
                                       blur_SSAO,
                                       shadow_texture);
        // 这里的好消息是 什么？ 这里可以申请；
        // 另一个消息是因为 移动到了这里的线程，那么是否就可以重新查找
        vk_render_queue::instance().execute_update_lambda();
    } {
        const auto view = Render_entt().view<Name_component>(); // 先用这里了，不应该，但是
        for (const auto it: view) {
            auto vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分
            Render_entt().emplace_or_replace<decltype(vk_descriptor_set)>(it, vk_descriptor_set);
        }
    }

    engine.update_bindless_descriptor_sets_function();
    object_parameter_update();
    descriptor_set_update_function();


    const uint64_t time_line             = Engine::get_current_submit_timeline();
    const VkCommandBuffer command_buffer = engine.get_current_command_buffer();
    engine.get_command_submit_manager().execute_callback_functions(time_line);
    engine.get_image_to_render();
    destroy_Render_entt();

    // 录制全部的绘制命令
    VCB vcb;
    vcb.reset_current_command_buffer(time_line, command_buffer);
    render_different_pass(vcb, engine, color_image, depth_image, depth_AO_image, depth_AO_copy_image, SSAO_image,
                          blur_SSAO_image, depth_shadow_image, entity_image, fxaa_result,
                          EASU_result,
                          RCAS_result,
                          compute_dof_blur_image);
    vcb.end_command_buffer();

    vcb.submit_render_queue(engine);
    engine.copy_image_to_screen();
    const uint64_t finished_timeline = engine.get_finished_timeline();
    clean_discard_vulkan_handle(finished_timeline);
}

void vk_render_GPU::exit_and_clean(VK_backend &backend) {
    vk_render_queue::instance().destroy();

    // 需要管理的资源以及删除的顺序
    // blender 中 descriptor_sets_layout 很有意思，在全局的最后才销毁 （中间申请的似乎从不销毁）
    // 一个原因是它关联了三个 内容，另一个原因是整体来说，它的布局很少改变，不会指数增长


    // 先删除（或重置）VkDescriptorSet，后删除 VkDescriptorSetLayout
    // 必须遵循“由实例到定义”的倒序销毁原则

    // 整体的顺序
    // descriptor_pools_
    // pipelines_
    // pipeline_layouts_
    // descriptor_sets_layout
    // shader_modules_
    // buffer_views_
    // buffers_
    // image_views_
    // images_

    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(VK_backend::instance().get_device()));

    Render_entt().clear();

    clean_discard_vulkan_handle(std::numeric_limits<uint64_t>::max());

    // destroy_descriptorPool();

    // 在开启多线程之前，先显式初始化这两个组件池 否则的话 还是有问题的
    // 如果两个线程同时第一次为一个新组件分配空间，会并发修改 registry 内部的总控结构，导致崩溃
    // registry.storage<Position>();
    // registry.storage<Velocity>();

    have_object_need_update = false;
    need_render             = not_start;
}

void vk_render_GPU::render_thread(VK_backend &backend, Engine &engine) {
    if (need_render == running) {
        return; // 已经在运行中了，直接返回
    }
    need_render = running; // 设置为运行中

    vk_render_queue::instance();
    // 这四个 我目前感觉是需要
    // 然后需要怎么做呢? VKR_Primitive 是基本的命令的合集
    // 想要一起绘制呢? 首先需要 把顶点 全部都绑定 到一起,之后  firstIndex 和  vertexOffset 需要 重新计算
    // index_type 也需要配合到一起,看看怎么工作 ,想简单一点,可以全部都变成 32位的索引
    // 然后其实还有一个实例化的问题,同一个渲染多次,那么创建多个 entity 吧
    // 能排列的在一起的话,就一起渲染,不能的话,那么久分析
    // Draw_command 提供 firstInstance  instanceCount
    // 单个绘制过程中不能更换 shader 那么最开始 设置的 material_index  需要替换掉 PBR_component
    // 也就是不能通过 firstInstance 直接得到需要的 material_index 的索引值,需要 查找一次
    // firstInstance  instanceCount 这两个其实就 只有第一个参数有用

    // 这里是渲染的部分, 那么需要什么呢?
    // 问题是需要存储什么?
    // Name_component
    // Proxy_descriptor_sets
    // tag
    // VKR_Render_state
    // Mesh_data
    // std::vector<VKR_Primitive>
    // std::vector<VKR_Render_state>
    // Shader_data
    // shader_constant_parameter
    // shader_need_parameter , 可能都能归类到这里
    // 包围盒
    // pbr 索引
    // 我还没有想好怎么压缩 它们
    // 这里排列的都是什么?  带几何的物体, 如果没有几何的物体呢?
    // 比如粒子等等


    FrameRate_measure framerate_measure(backend.get_refresh_rate());
    while (need_render == running) {
        framerate_measure.begin_frame();
        engine.set_framerate(framerate_measure.get_frame_rate());
        render_once(backend, engine);
        framerate_measure.end_frame();
    }
    exit_and_clean(backend);
}

void vk_render_GPU::render_thread_stop() {
    if (need_render == running) {
        need_render = need_stop;
    }
}

void vk_render_GPU::render_thread_stop_and_wait() {
    if (need_render == running) {
        need_render = need_stop;
        while (need_render != not_start) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

vk_render_GPU &vk_render_GPU::instance() {
    static vk_render_GPU *instance = nullptr;
    static std::once_flag flag;
    std::call_once(flag, []() {
        instance = new vk_render_GPU();
    });
    return *instance;
}

void vk_render_GPU::clean_discard_vulkan_handle(const uint64_t finished_timeline) {
    discard_buffer_map_clean(finished_timeline);         //  buffers_
    discard_image_and_view_map_clean(finished_timeline); //  image_views_
}

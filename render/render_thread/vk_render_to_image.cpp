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
#include "vulkan_render_manage.h"
#include "time_measure.h"
#include "transform_component.h"
#include "vulkan_execute_command.h"


void render_different_pass(VCB &vcb,
                           Engine &engine,
                           VKR_image_ptr color_image,
                           VKR_image_ptr depth_image,
                           VKR_image_ptr depth_AO_image,
                           VKR_image_ptr entity_image
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
    // 视锥裁剪
    {
        auto view = Render_entt().view<GPU_frustum_cull>();
        for (const auto entity: view)
            vcb.calculate_frustum_cull(entity, frustum_planes);
    }
    // 阴影的 pass
    {
        // g_buffer_image_indices 这是需要看看怎么传递进入其中
        const auto view = Render_entt().view<shadow_pass_tag>();
        if (!view.empty()) {
            vcb.begin_shadow_pass(depth_image);

            // 中间需要添加 被光 照 到的物体，能产生阴影的物体
            // 这里的时候发生了一点改变，为什么呢？ 单个 mesh 需要多个不同的 render pass
            // 这也意味着 需要 它 开始需要多个 shader 了
            // 然后的问题就是，多个 shader 中，哪些是共同的部分，哪些不是？
            // 这里开始需要什么了呢？ 光，灯光是需要的部分 ，物体在世界空间的位姿也是需要的部分，
            // 不同的是输出的内容，以及输出的内容在什么时候再次被需要
            // model_matrix 是 在多个 shader 都保持不变的值。

            // 拿到z值，并反推出在世界空间中的位置需要 view 和 projection 的逆矩阵
            // 每个灯光，都需要其 view 和 projection 矩阵
            // 这个时候，其实应该是可以考虑关于遮挡部分的内容了，比如，
            // 计算平头截体，然后将不在其中的物体给排除出去。
            // 这个时候需要什么呢？ 物体的包围盒，model ,之后 再与 平头截体进行相交的判断
            // 之后再是什么呢？ 看看如何将这部分的计算放到GPU中计算

            vcb.end_rendering();
            vcb.shadow_pass_barrier();
        }
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

    // 这里是绘制 不透明
    // 不能按照
    {
        vcb.begin_rendering_depth_attachment(depth_AO_image,
                                             VK_ATTACHMENT_LOAD_OP_CLEAR);
        auto view = Render_entt().view<opacity_gltf_tag, GPU_frustum_cull, Name_component>();
        for (const auto entity: view) {
            auto command_calculate      = Render_entt().get<GPU_frustum_cull>(entity);
            auto name                   = Render_entt().get<Name_component>(entity);
            const auto &shader_data_ref =
                    engine.get_shader_manager().find(VKR_shader_paths{
                                                         "opacity_depth_write",
                                                         "opacity_depth_write",
                                                         "",
                                                         "",
                                                         VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                                         VK_FORMAT_D32_SFLOAT
                                                     });
            vcb.bind_pipeline_update_parameter(entity, shader_data_ref);
            // 现在绑定的管线是有问题的,
            vcb.default_status();

            vcb.DrawIndexedIndirect(entity, command_calculate);
        }
        vcb.end_rendering();
        vcb.current_write_next_read_depth({depth_AO_image});
    }

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
                vcb.DrawIndexedIndirect(entity, command_calculate);
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
    }


    // 在这里的时候需要插入 FXAA
    {
        vcb.current_write_next_read_image({
                                              color_image, entity_image
                                          });
        vcb.begin_rendering_attachment(engine.get_current_swap_chain_image(),
                                       depth_image,
                                       VK_ATTACHMENT_LOAD_OP_CLEAR); {
            auto command_shader = engine.get_shader_manager().get_offscreen_to_screen_shader_data();
            vcb.render_post_deal(command_shader, entt::null);
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, UI_2D_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, Line_tag>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, imgui_draw>();
            for (const auto entity: view) {
                vcb.build_draw_command(entity);
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
    const auto color_image    = engine.get_image_manager().get_one_color_image();
    const auto entity_image   = engine.get_image_manager().get_one_entity_image();
    const auto depth_image    = engine.get_image_manager().get_one_depth_image();
    const auto depth_AO_image = engine.get_image_manager().get_one_depth_AO_image(); {
        std::unique_lock<std::mutex> lock(mtx);

        auto offscreen = create_2d_texture(color_image);
        auto depth     = create_2d_texture(depth_AO_image);

        engine.update_global_parameter(offscreen, {}, depth); // 这里的好消息是 什么？ 这里可以申请；
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
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    engine.get_image_to_render();
    destroy_Render_entt();

    // 录制全部的绘制命令
    VCB vcb;
    vcb.reset_current_command_buffer(time_line, command_buffer);
    render_different_pass(vcb, engine, color_image, depth_image, depth_AO_image, entity_image);
    vcb.end_command_buffer();

    engine.submit_render_queue(time_line);
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
    Render_entt().group<PBR_material_index, Transform_Matrix, Render_AABB, Draw_command>();
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

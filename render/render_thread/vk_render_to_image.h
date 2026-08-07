//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_VK_RENDER_TO_IMAGE_H
#define HELLO_MAC_VK_RENDER_TO_IMAGE_H
#include <atomic>
#include <mutex>
#include <thread>

#include "create_pipeline.h"
#include "shader_create.h"
#include "../engine.h"
#include "transfer_texture_to_gpu.h"
#include "vertex_and_buffer_index.h"
#include "vulkan_build_command_buffer.h"
#include "vulkan_backend.h"
#include "vulkan_texture_bindless.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#include "Command_calculate.h"
#include "descriptor_pool.h"
#include "framerate_measure.h"
#include "pipeline_layout.h"
#include "pipeline_layout_component.h"
#include "pipeline_component.h"
#include "name_component.h"
#include "vulkan_render_manage.h"
#include "sets_and_bindings_layout.h"
#include "time_measure.h"
#include "transform_component.h"

struct float4 {
    float x, y, z, w;
};

struct FrustumCorners {
    float4 corners[8];
};

bool frustum_cull(const FrustumPlanes &frustum_planes,
                  const AABB_min_max<Point_3> &bounds,
                  const Eigen::Vector4f &camera_pos);

bool frustum_cull_2(const FrustumPlanes &frustum_planes,
                    const Render_AABB &bounds,
                    const Eigen::Vector4f &camera_pos);

struct ViewCullingData {
    /** \note float3 array padded to float4. */
    /** Frustum corners. */
    FrustumCorners frustum_corners;
    FrustumPlanes frustum_planes;
    float4 bound_sphere;
};


class vk_render_GPU {
    mutable std::mutex mtx;
    std::atomic<bool> have_object_need_update = false;
#define not_start 0
#define running 1
#define need_stop 2
    std::atomic<uint32_t> need_render = not_start; // 这里状态有点少了，需要 未开始，运行中，需停止


public:
    void one_cycle(VK_backend &handle) {
        VK_backend::instance().update_current_extent();
        auto &engine = Engine::instance(); {
            std::unique_lock<std::mutex> lock(mtx);
            engine.update_global_parameter(); // 这里的好消息是 什么？ 这里可以申请；
            // 另一个消息是因为 移动到了这里的线程，那么是否就可以重新查找
            vk_render_queue::instance().execute_update_lambda();
        } {
            const auto view = Render_entt().view<Name_component>(); // 先用这里了，不应该，但是
            for (const auto it: view) {
                auto vk_descriptor_set = get_descriptor_sets(it); // 唯一有可能每帧更新的部分
                Render_entt().emplace_or_replace<decltype(vk_descriptor_set)>(it, vk_descriptor_set);
            }
        }
        // bindless_uniform_sampler2D_update_function();
        // global_uniform_buffer_update_function();
        Engine::instance().update_bindless_descriptor_sets_function();
        object_parameter_update();
        descriptor_set_update_function();
        push_constant_update_function();
        //
        {
            const auto view = Render_entt().view<Render_destroy_tag>();
            Render_entt().destroy(view.begin(), view.end()); // 执行销毁程序
        }
        const VkQueryPool queryPool = VK_NULL_HANDLE;

        Engine::instance().get_image_to_render(); // 这里已经有完整的
        const uint64_t time_line = Engine::get_current_submit_timeline();
        // 查出哪些物体是需要绘制的，但是命令是需要看阶段的
        reset_current_command_buffer(handle, queryPool, time_line);


        auto frustum_planes = Engine::instance().get_frustum_planes();
        auto camera_pos     = Engine::instance().get_world_camera_pos();
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
            for (const auto it: view) {
                // 这里还需要改为 dispatch
                build_compute_dispatch(handle, it, time_line);
            }
            // add_one_indirect_draw_barrier(handle,VK_NULL_HANDLE, 1024);
        } {
            const auto cb = Engine::instance().get_current_command_buffer();
            auto view     = Render_entt().view<Command_calculate>();
            // 这里需要做什么呢? 创建计算着色器
            // 计算AABB 包围盒 将新的 command 写入需要更改的 位置中
            // 添加 屏障
            // 绘制调用新的绘制命令
            for (const auto it: view) {
                auto command_shader = Engine::instance().get_command_calculate_shader_data();
                vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, command_shader->pipeline_t);

                auto command_calculate           = Render_entt().get<Command_calculate>(it);
                command_calculate.frustum_planes = frustum_planes; // 还需要在这里更新一次
                vkCmdPushConstants(cb, command_shader->pipeline_layout,
                                   VK_SHADER_STAGE_COMPUTE_BIT, 0, 116,
                                   &command_calculate);
                vkCmdDispatch(cb, ALIGN_256(command_calculate.command_size) / 256, 1, 1);

                auto &parameter               = Render_entt().get_or_emplace<shader_need_parameter>(it);
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
                        .buffer = command_buffer->get_buffer_handle(time_line), // 这里的buffer 句柄 应该从哪里拿?
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


        // 阴影的 pass
        {
            // g_buffer_image_indices 这是需要看看怎么传递进入其中
            const auto view = Render_entt().view<shadow_pass_tag>();
            if (!view.empty()) {
                begin_shadow_pass(handle, time_line);

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

                end_rendering(handle);
                shadow_pass_barrier(handle, time_line);
            }
        }
        auto g_buffer_image_indices = begin_g_buffer_rendering_attachment(handle, time_line); {
            auto view = Render_entt().view<deferred_pass_tag>();
            if (!view.empty()) {
                auto view_opacity = Render_entt().view<opacity_tag, Name_component>();
                for (const auto it: view_opacity) {
                    auto name = Render_entt().get<Name_component>(it);
                    build_command_buffer(handle, it, time_line);
                }
            }
        }

        end_rendering(handle);
        g_buffer_attachment_barrier(handle, time_line);


        begin_rendering_attachment(handle, time_line); // 好消息是自己原本的理解已经基本成型了，坏消息是我没有确定分离的位置。
        // 应该先划分不同的 pass 阶段，
        //  deferred  不应该将深度值写入的
        {
            // g_buffer_image_indices 这是需要看看怎么传递进入其中
            auto view = Render_entt().view<deferred_pass_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            // 按照常理来说，包围盒的时候 深度比较出问题了，所以会覆盖
            auto view = Render_entt().view<std::vector<VKR_Primitive>, skybox_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<opacity_tag, Command_calculate, Name_component>();
            for (const auto it: view) {
                // 这里需要做什么呢? 创建计算着色器
                // 计算AABB 包围盒 将新的 command 写入需要更改的 位置中
                // 添加 屏障
                // 绘制调用新的绘制命令
                auto command_calculate = Render_entt().get<Command_calculate>(it);
                auto name              = Render_entt().get<Name_component>(it);
                bind_pipeline_update_parameter(handle, it, time_line);
                DrawIndexedIndirect(handle, it, command_calculate, time_line);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>,
                                           opacity_tag,
                                           Name_component>();
            for (const auto it: view) {
                auto name = Render_entt().get<Name_component>(it);
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, translate_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<volume_pass_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, UI_2D_tag>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        } {
            auto view = Render_entt().view<std::vector<VKR_Primitive>, imgui_draw>();
            for (const auto it: view) {
                build_command_buffer(handle, it, time_line);
            }
        }

        end_rendering(handle);


        end_command_buffer(handle, queryPool, time_line);

        engine.submit_render_queue(time_line);
        engine.copy_image_to_screen();


        // render_object_function();
        clean_need_objects();
        //
    }

    void exit_and_clean(VK_backend &handle) {
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

        clean_need_objects();

        // destroy_descriptorPool();


        // pipeline 建议提前清理
        clean_all_pipeline(handle);
        clean_all_pipeline_layout(handle);
        clean_all_shader_object(handle);
        // VkDescriptorSet
        clean_all_descriptor_sets_layout(handle);

        // clean_all_mesh_object(); // 放在这里似乎并不是太好， 函数被清理了

        // destroy_texture();


        have_object_need_update = false;
        need_render             = not_start;
    }


    void render_thread(VK_backend &handle) {
        if (need_render == running) {
            return; // 已经在运行中了，直接返回
        }
        need_render = running; // 设置为运行中
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

        FrameRate_measure framerate_measure(VK_backend::instance().get_refresh_rate());
        while (need_render == running) {
            framerate_measure.begin_frame();
            Engine::instance().set_framerate(framerate_measure.get_frame_rate());
            one_cycle(handle);
            framerate_measure.end_frame();
        }
        exit_and_clean(handle);
    }

    // 显式同步：即使使用 detach，也应通过原子变量（如 std::atomic<bool>）或信号量
    // 通知子线程退出，并确保其在主线程销毁全局资源前完成清理
    void render_thread_stop() {
        if (need_render == running) {
            need_render = need_stop;
        }
    }

    void render_thread_stop_and_wait() {
        if (need_render == running) {
            need_render = need_stop;
            while (need_render != not_start) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    static vk_render_GPU &instance() {
        static vk_render_GPU *instance = nullptr;
        static std::once_flag flag;
        std::call_once(flag, []() {
            instance = new vk_render_GPU();
        });
        return *instance;
    }

private:
    void clean_need_objects() {
        // discard_descriptor_set_map_clean(); // descriptor_pools_
        //                                     pipelines_
        //                                     pipeline_layouts_
        //                                     descriptor_sets_layout
        //                                     shader_modules_
        //                                     buffer_views_
        discard_buffer_map_clean();         //         buffers_
        discard_image_and_view_map_clean(); //  image_views_
        //                                     images_


        // 简单的将内存区域标记为没有内容
        // init_need_objects 再根据需要进行移动或者拼接操作
    }

private:
    vk_render_GPU() {
    }

    ~vk_render_GPU() = default;

public:
    vk_render_GPU(const vk_render_GPU &) = delete;

    vk_render_GPU &operator=(const vk_render_GPU &) = delete;

    vk_render_GPU(vk_render_GPU &&) = delete;

    vk_render_GPU &operator=(vk_render_GPU &&) = delete;
};


#endif //HELLO_MAC_VK_RENDER_TO_IMAGE_H

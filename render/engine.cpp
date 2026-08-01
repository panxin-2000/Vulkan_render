//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include "vulkan_code/sets_and_bindings_layout.h"
#include "shader_component.h"
#include "transform_component.h"
#include "vulkan_code/vulkan_backend.h"
#include "vulkan_code/vulkan_sample.h"


static std::atomic<Engine *> engine_instance{nullptr};

Engine &Engine::instance() {
    // 1. 第一次读取（使用 Acquire 保证能看到初始化后的完整内存）
    Engine *current = engine_instance.load(std::memory_order_acquire);
    if (current == nullptr) {
        // 2. 抢输了的线程，或者刚进来的线程，都在这里准备
        const auto new_value = new Engine();

        // 【关键修复】：在把指针暴露给全局之前，在线程私有空间内彻底把句柄初始化好！
        new_value->create();

        Engine *expected = nullptr;
        // 3. 经典的无锁自旋尝试
        // 如果 instance 是 expected(nullptr)，就写入 new_value
        if (engine_instance.compare_exchange_strong(expected, new_value,
                                                    std::memory_order_release,
                                                    std::memory_order_acquire)) {
            // 抢赢了！
            current = new_value;
        } else {
            // 抢输了！说明别的线程已经把一个【完全初始化好】的单例塞进 instance 了
            // new_value();
            new_value->destroy();
            delete new_value;   // 销毁自己这个备胎
            current = expected; // expected 已经被 CAS 自动更新为抢赢线程的那个完整指针
        }
    }
    return *current;
}


void Engine::get_query_results() {
    const auto &backend = VK_backend::instance();
    if (get_current_query_pool() != VK_NULL_HANDLE) {
        uint64_t timestamps[2]; // 准备接收数组
        VkResult result = vkGetQueryPoolResults(
                                                backend.get_device(),
                                                get_current_query_pool(),
                                                0,                  // 从 index 0 开始
                                                2,                  // 获取 2 个结果
                                                sizeof(timestamps), // 总大小 16 字节
                                                timestamps,         // 目标数组
                                                sizeof(uint64_t),   // 每个元素的步长
                                                VK_QUERY_RESULT_64_BIT
                                               );
        if (result == VK_SUCCESS) {
            uint64_t start = timestamps[0];
            uint64_t end   = timestamps[1];
            // 计算耗时 (ns) = (end - start) * timestampPeriod
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        } else if (result == VK_NOT_READY) {
        }
    }
}

void Engine::create_query_pool() {
    const auto &backend = VK_backend::instance();
    VkQueryPoolCreateInfo queryPoolInfo{};
    queryPoolInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP; // 指定为时间戳类型
    queryPoolInfo.queryCount = 2;                       // 比如：一个存起点，一个存终点
    for (auto i = 0; i < maxFramesInFlight; i++) {
        if (vkCreateQueryPool(backend.get_device(), &queryPoolInfo, nullptr, &query_pools[i]) != VK_SUCCESS) {
            LOG_INFO(g_log(), "vkCreateQueryPool failed");
        } else {
            // vkResetQueryPool(handle.device_, queryPool, 0, 2);
            // 需要 VK_EXT_host_query_reset 的扩展
        }
    }
}

void Engine::destroy_query_pool() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyQueryPool(backend.get_device(), query_pools[i], nullptr);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_command_buffer() {
    const auto &backend = VK_backend::instance();

    VkCommandBufferAllocateInfo cbAllocCI{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = get_command_pool(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = maxFramesInFlight
    };
    VK_CHECK_RESULT_NOT_EXIT(vkAllocateCommandBuffers(backend.get_device(), &cbAllocCI,
                                 command_buffers_.data()));
}

void Engine::destroy_command_buffer() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkFreeCommandBuffers(backend.get_device(), get_command_pool(), 1, &command_buffers_[i]);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_fences() {
    const auto &backend = VK_backend::instance();
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(backend.get_device(), &fenceCI, nullptr, &fences_[i]));
    }
}

void Engine::destroy_fences() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyFence(backend.get_device(), fences_[i], nullptr); //  这里还需要
        fences_[i] = VK_NULL_HANDLE;
    }
}

void Engine::create_present_Semaphores() {
    const auto &backend = VK_backend::instance();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(backend.get_device(), &semaphoreCI,
                                     nullptr, &present_semaphores_[i]));
    }
}

void Engine::destroy_present_Semaphores() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroySemaphore(backend.get_device(), present_semaphores_[i], nullptr); //
        present_semaphores_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_renderSemaphores() {
    const auto &backend = VK_backend::instance();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    render_to_image_semaphores_.resize(get_swap_chain_images().size());
    LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
    for (auto &semaphore: render_to_image_semaphores_) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(backend.get_device(), &semaphoreCI, nullptr, &semaphore));
    }
}

void Engine::destroy_renderSemaphores() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
        vkDestroySemaphore(backend.get_device(), render_to_image_semaphores_[i], nullptr);
        render_to_image_semaphores_[i] = VK_NULL_HANDLE;
    }
}


void Engine::destroy_and_recreate_fence_and_semaphore() {
    destroy_fences();
    destroy_present_Semaphores();
    destroy_renderSemaphores();
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
    imageIndex = 0;
    frameIndex = 0;
}

void Engine::destroy_render_image() {
    for (const auto &image: depth_images_) {
        image->destroy_image();
    }
    depth_images_.clear();
    for (const auto &image: swap_chain_images_) {
        image->destroy_image();
    }
    swap_chain_images_.clear();
    for (const auto &image: G_buffer_Position_images_) {
        image->destroy_image();
    }
    G_buffer_Position_images_.clear();
    for (const auto &image: g_buffer_Normal_images_) {
        image->destroy_image();
    }
    g_buffer_Normal_images_.clear();
    for (const auto &image: G_buffer_BaseColor_images_) {
        image->destroy_image();
    }
    G_buffer_BaseColor_images_.clear();
}

void Engine::create_render_image() {
    swap_chain_images_ = VK_backend::instance().create_swap_chain_image_and_view();

    depth_images_.push_back(VK_backend::instance().create_depth_image_and_view());
    depth_images_.push_back(VK_backend::instance().create_depth_image_and_view());
    depth_images_.push_back(VK_backend::instance().create_depth_image_and_view());

    G_buffer_Position_images_.push_back(VK_backend::instance().
                                        create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    G_buffer_Position_images_.push_back(VK_backend::instance().
                                        create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    g_buffer_Normal_images_.push_back(VK_backend::instance().
                                      create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    g_buffer_Normal_images_.push_back(VK_backend::instance().
                                      create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    G_buffer_BaseColor_images_.push_back(VK_backend::instance().create_G_buffer_image_and_view(VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    G_buffer_BaseColor_images_.push_back(VK_backend::instance().create_G_buffer_image_and_view(VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
}


void Engine::create() {
    descriptor_pool_manager_.create(); {
        VKR_shader_paths shader_paths{
            "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.vert.spv",
            "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
            "", ""
        };
        shader_date = VKR_shader_init(shader_paths);
    } {
        VKR_shader_paths shader_paths{
            "",
            "",
            "",
            "/Users/panxin/CLionProjects/hello_mac/render/shader/command_calculate.comp.spv"
        };
        command_calculate = VKR_shader_init(shader_paths);
    }
    descriptor_pool_manager_.set_shader_data(shader_date);
    bindless_descriptor_sets_ =
            descriptor_pool_manager_.allocate_bindless_descriptor_sets(
                                                                       shader_date->bindless_sets_bindings,
                                                                       shader_date->bindless_set_layout);
    // 需要在这里创建一些内容
    // 或者说，到这里之后才能够进行上传
    {
        // 添加一张纯白的背景图片
        auto texture_default_color_ = create_single_color_texture(0xff, 0xff, 0xff);
        add_bindless_texture(texture_default_color_);
        // 默认 指向于 z 轴的 法线
        auto texture_default_normal_ = create_single_color_texture(128, 128, 255);
        add_bindless_texture(texture_default_normal_);
        PBR_Texture_ptr ptr = {
            texture_default_color_,
            texture_default_normal_,
            {},
            {}
        };
        pbr_manager_.push({}, ptr);
    }
    // {
    //     // std::optional<Texture_parameter> texture = create_texture_from_image( );
    //     // uint32_t index                           = add_bindless_uniform_sampler2D("default_text_MSDF_texture", texture);
    //     // assert(index == 1);
    // }


    create_render_image();
    create_command_pool();
    create_command_buffer();
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
    create_timeline_Semaphores();
}


void Engine::add_bindless_texture(const std::optional<Texture_parameter> &texture) {
    for (auto const &[set_value, bindings_map]: shader_date->bindless_sets_bindings) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == "bindless_samplerColorMap") {
                auto index                                              = texture.value().image.get_index();
                Update_descriptor_binding temp                          = {};
                temp.binding_name                                       = "bindless_samplerColorMap";
                temp.resource_type                                      = "uniform sampler2D";
                temp.dstSet                                             = set_value;
                temp.descriptor_write_binding.sType                     = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                temp.descriptor_write_binding.dstBinding                = binding_value;
                temp.descriptor_write_binding.dstArrayElement           = index;
                temp.descriptor_write_binding.descriptorCount           = 1;
                temp.descriptor_write_binding.pBufferInfo               = nullptr;
                temp.descriptor_write_binding.pImageInfo                = nullptr;
                temp.descriptor_write_binding.pTexelBufferView          = nullptr;
                temp.descriptor_write_binding.descriptorType            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                temp.texture_info                                       = {true, texture.value()};
                update_bindless_descriptor_sets_[std::to_string(index)] = temp;
                // 这个时候需要做什么呢？ 添加一个更新的函数，这是记录了需要更新的内容，还没有真正更新
                // bindless.bindings[name] = {return_value, temp}; // 这句应该是暂时没有用了
            }
        }
    }
}

void Engine::update_bindless_descriptor_sets_function() {
    update_descriptor_sets(update_bindless_descriptor_sets_, bindless_descriptor_sets_);
    update_bindless_descriptor_sets_.clear();
}


void Engine::recreate_swap_chain() {
    VK_backend::instance().set_frame_buffer_resize(false);
    const auto old_swap_chain = VK_backend::instance().get_swap_chain();
    VK_backend::instance().create_swap_chain(old_swap_chain);
    destroy_render_image();
    create_render_image();
    VK_backend::instance().destroy_swap_chain(old_swap_chain);
}

void Engine::destroy() {
    pbr_manager_.destroy();
    if (pbr_components_buffer_ != nullptr) {
        pbr_components_buffer_ = {};
    }
    update_bindless_descriptor_sets_.clear();
    const auto &backend = VK_backend::instance();
    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(backend.get_device()));

    // 这里的顺序不对
    destroy_render_image();
    discard_buffer_map_clean();
    discard_image_and_view_map_clean();

    // 销毁 timeline_semaphore 再全部检查一遍再销毁
    destroy_all_vulkan_sample();


    vkDestroySemaphore(VK_backend::instance().get_device(), vk_timeline_semaphore_, nullptr);
    vk_timeline_semaphore_ = VK_NULL_HANDLE;


    destroy_fences();
    destroy_present_Semaphores();
    destroy_renderSemaphores();
    destroy_command_buffer();
    destroy_command_pool();
    descriptor_pool_manager_.destroy();
}

void Engine::destroy_command_pool() {
    const auto &backend = VK_backend::instance();
    for (auto command_pool: command_pools_) {
        if (command_pool != VK_NULL_HANDLE)
            vkDestroyCommandPool(backend.get_device(), command_pool, nullptr);
    }
}

void Engine::create_command_pool() {
    // Command pool
    const auto &backend = VK_backend::instance();
    const VkCommandPoolCreateInfo commandPoolCI{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = backend.get_queue_Family()
    };
    VkCommandPool commandPool;
    VK_CHECK_RESULT(vkCreateCommandPool(backend.get_device(), &commandPoolCI, nullptr, &commandPool));
    command_pools_.push_back(commandPool);
}


void Engine::update_global_pbr_parameter(
    std::map<std::string, Update_descriptor_binding> &update_global_descriptor_sets) {
    auto size = pbr_manager_.size() * sizeof(PBR_component);
    if (size > 0) {
        auto src = pbr_manager_.data();

        if (pbr_components_buffer_ == nullptr) {
            pbr_components_buffer_ = create_SSBO_buffer(ALIGN_1024(size * 2));
        }
        if (pbr_components_buffer_.get()->complete_size() < size) {
            pbr_components_buffer_ = create_SSBO_buffer(ALIGN_1024(size * 2));
        }
        auto mem_copy_function = [src,size](void *dst) {
            memcpy(dst, src, size);
        };


        copy_mem_from_cpu_to_gpu(pbr_components_buffer_, mem_copy_function);
        set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                             "global_PBR_parameters", pbr_components_buffer_);
    }
}


#include <vector>
#include <Eigen/Dense>


void ExtractVulkanFrustumPlanes(const Eigen::Matrix4f &projection,
                                const Eigen::Matrix4f &view,
                                FrustumPlanes &frustum_planes) {
    // 1. 计算 View-Projection 复合矩阵
    Eigen::Matrix4f vp = projection * view;

    // 2. 提取 4 个行向量 (Row Vectors)
    Eigen::Vector4f r0 = vp.row(0);
    Eigen::Vector4f r1 = vp.row(1);
    Eigen::Vector4f r2 = vp.row(2);
    Eigen::Vector4f r3 = vp.row(3);

    // 声明 6 个平面的临时存储
    std::vector<Eigen::Vector4f> raw_planes(6);

    // 3. 【Vulkan 专属公式组合】
    raw_planes[0] = r3 + r0; // 左平面 (Left)
    raw_planes[1] = r3 - r0; // 右平面 (Right)
    raw_planes[2] = r3 + r1; // 下平面 (Bottom)
    raw_planes[3] = r3 - r1; // 上平面 (Top)
    raw_planes[4] = r2;      // 近平面 (Near): Vulkan 裁剪空间中 z = 0
    raw_planes[5] = r3 - r2; // 远平面 (Far): Vulkan 裁剪空间中 z = w


    // 4. 对 6 个平面进行严谨的数学归一化
    for (int i = 0; i < 6; ++i) {
        // 提取平面法向量 (A, B, C) 并计算模长
        float length = raw_planes[i].head<3>().norm();

        // 防除以 0 保护（针对退化矩阵）
        float inv_length = (length > 0.0f) ? (1.0f / length) : 1.0f;

        // 归一化整个 vec4 (A, B, C, D)
        frustum_planes.planes[i] = raw_planes[i] * inv_length;
    }
}


bool frustum_cull(const FrustumPlanes &frustum_planes,
                  const AABB_min_max<Point_3> &bounds,
                  const Eigen::Vector4f &camera_pos) {
    // 根据 视锥裁切平面 法向量 ， 找到 包围盒 中 距离 平面最近的点， 判断 是否在视锥范围内
    Eigen::Vector3f hi{bounds.max_point_.x, bounds.max_point_.y, bounds.max_point_.z};
    Eigen::Vector3f lo{bounds.min_point_.x, bounds.min_point_.y, bounds.min_point_.z};

    bool is_camera_inside = (camera_pos.x() >= lo.x() && camera_pos.x() <= hi.x()) &&
                            (camera_pos.y() >= lo.y() && camera_pos.y() <= hi.y()) &&
                            (camera_pos.z() >= lo.z() && camera_pos.z() <= hi.z());

    if (is_camera_inside) {
        return true; // 🌟 相机在物体内部，绝对可见，直接熔断返回！
    }

    float is_visible = 1.0f;
    for (int i = 0; i < 6; i++) {
        // 获取当前平面的系数。p.xyz 是法向量，p.w 是从原点到平面的距离
        Eigen::Vector4f p         = frustum_planes.planes[i];
        auto p_xyz                = p.head<3>();
        auto high_mask            = (p_xyz.array() > 0.0f);
        Eigen::Vector3f max_coord = high_mask.select(hi, lo);
        float distance            = max_coord.dot(p.head<3>()) + p.w();
        is_visible                *= (distance >= -0.0001f) ? 1.0f : 0.0f;
    }
    return is_visible > 0.5f;
}

bool frustum_cull_2(const FrustumPlanes &frustum_planes,
                    const Render_AABB &bounds,
                    const Eigen::Vector4f &camera_pos) {
    // 包围盒的
    // bounds.centroid_points      最后一个分量为1
    // bounds.direction_intervals  最后一个分量为0
    bool all_planes_inside = true;

    // 强制编译器展开循环，消除循环开销
#pragma unroll
    for (int i = 0; i < 6; ++i) {
        const Eigen::Vector4f &p = frustum_planes.planes[i];
        // 2. 【核心优化】计算 AABB 沿平面法线的最大正向投影半径
        // .cwiseAbs() 会对法向量的每个分量取绝对值
        // .dot() 执行极致的 SIMD 乘加运算，彻底代替了原本的 mix 掩码操作
        float projectedRadius = bounds.direction_intervals.dot(p.cwiseAbs());
        // 3. 计算中心点到平面的带符号物理距离
        const float distanceToCenter = bounds.centroid_points.dot(p);
        if (distanceToCenter < -projectedRadius - 0.0001f) {
            return false; // 整个盒体完全在平面外侧，安全剔除  // 有时很快,有时很慢, 是因为这里有快捷返回
        }
    }
    // 如果 6 个平面都认为盒子完全在内侧，返回 1，否则返回 2（相交）
    return true;
}


void Engine::update_global_parameter() {
    global_descriptor_sets_ = descriptor_pool_manager_.allocate_global_descriptor_sets(
         shader_date->global_sets_bindings,
         shader_date->global_descriptor_sets_layout
        );
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    const auto extent              = VK_backend::instance().get_current_extent();
    global_parameters_.screen_size = {static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 0};

    ExtractVulkanFrustumPlanes(global_parameters_.projection_matrix,
                               global_parameters_.view_matrix,
                               global_parameters_.frustum_planes);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_parameters", global_parameters_);

    update_global_pbr_parameter(update_global_descriptor_sets);
    Proxy_descriptor_sets descriptor_sets; // 这里是需要按照顺序的
    auto bindless_descriptor_sets = get_bindless_descriptor_set();
    auto global_descriptor_sets   = get_global_descriptor_set();
    // 先使用下面的直接引用，之后再看怎么获取父节点的全局索引
    // auto &global_descriptor_sets = vk_s_d_s->global_descriptor_sets;
    descriptor_sets.reserve(bindless_descriptor_sets.size() +
                            global_descriptor_sets.size());
    descriptor_sets.insert(descriptor_sets.end(),
                           bindless_descriptor_sets.begin(),
                           bindless_descriptor_sets.end());
    descriptor_sets.insert(descriptor_sets.end(),
                           global_descriptor_sets.begin(),
                           global_descriptor_sets.end());
    update_descriptor_sets(update_global_descriptor_sets, descriptor_sets);
}

void Engine::update_bindless_parameter() {
    update_descriptor_sets(update_bindless_descriptor_sets_, bindless_descriptor_sets_);
    // 看起来确实很简单，只有这一个函数
    // 某些内容写过一次之后是不需要去重新再去写的
}


Proxy_descriptor_sets Engine::get_bindless_descriptor_set(const uint index) {
    return bindless_descriptor_sets_;
}

Proxy_descriptor_sets Engine::get_global_descriptor_set(const uint index) {
    return global_descriptor_sets_;
}

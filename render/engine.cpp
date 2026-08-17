//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include <sys/stat.h>

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
            delete new_value;   // 销毁自己这个备胎
            current = expected; // expected 已经被 CAS 自动更新为抢赢线程的那个完整指针
        }
    }
    return *current;
}

uint64_t Engine::get_finished_timeline() const {
    uint64_t current_timeline;
    // todo: 偶尔出现一个这个错误，应该是两个线程之间的一个同步问题
    // 确定一下 这个 can't be called on VkImageView 出现后才会出现  assert 失败的情况
    // vkGetSemaphoreCounterValue(): semaphore Invalid VkSemaphore Object 0x0
    VkResult result = vkGetSemaphoreCounterValue(VK_backend::instance().get_device(), vk_timeline_semaphore_,
                                                 &current_timeline);
    assert(result == VK_SUCCESS && "vulkan get timeline semaphore value error");
    return current_timeline;
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
    for (const auto &image: swap_chain_images_) {
        image->destroy_image();
    }
    swap_chain_images_.clear();
    render_image_manager_.destroy();
}

void Engine::create_render_image() {
    swap_chain_images_ = VK_backend::instance().create_swap_chain_image_and_view();


    render_image_manager_.create();
}


void Engine::create() {
    descriptor_pool_manager_.create();
    command_submit_manager_.create();
    shader_manager_.create();
    auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
    descriptor_pool_manager_.set_shader_data(gltf_shader_data);
    bindless_descriptor_sets_ =
            descriptor_pool_manager_.allocate_bindless_descriptor_sets(
                                                                       gltf_shader_data->bindless_sets_bindings,
                                                                       gltf_shader_data->bindless_set_layout);
    global_descriptor_sets_[0] = descriptor_pool_manager_.allocate_global_descriptor_sets(
         gltf_shader_data->global_sets_bindings,
         gltf_shader_data->global_descriptor_sets_layout
        );
    global_descriptor_sets_[1] = descriptor_pool_manager_.allocate_global_descriptor_sets(
         gltf_shader_data->global_sets_bindings,
         gltf_shader_data->global_descriptor_sets_layout
        );
    global_descriptor_sets_[2] = descriptor_pool_manager_.allocate_global_descriptor_sets(
         gltf_shader_data->global_sets_bindings,
         gltf_shader_data->global_descriptor_sets_layout
        );

    // 需要在这里创建一些内容
    // 或者说，到这里之后才能够进行上传
    {
        // 添加一张纯白的背景图片
        auto texture_default_color_ = create_single_color_texture(0x7f, 0x7f, 0x7f);
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
    auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
    for (auto const &[set_value, bindings_map]: gltf_shader_data->bindless_sets_bindings) {
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
    shader_manager_destroy();

    pbr_manager_.destroy();
    command_submit_manager_.destroy();
    if (pbr_components_buffer_ != nullptr) {
        pbr_components_buffer_ = {};
    }
    update_bindless_descriptor_sets_.clear();
    const auto &backend = VK_backend::instance();
    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(backend.get_device()));

    // 这里的顺序不对
    destroy_render_image();
    // 需要强制清除
    constexpr uint64_t finished_timeline = std::numeric_limits<uint64_t>::max();
    discard_buffer_map_clean(finished_timeline);
    discard_image_and_view_map_clean(finished_timeline);

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
        auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
        set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                             "global_PBR_parameters", pbr_components_buffer_);
    }
}


void Engine::update_global_parameter(std::optional<Texture_parameter> offscreen,
                                     std::optional<Texture_parameter> SSAO,
                                     std::optional<Texture_parameter> depth) {
    auto gltf_shader_data        = shader_manager_.get_gltf_shader_data();
    global_descriptor_sets_index = (global_descriptor_sets_index + 1) % 3;
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    const auto extent              = VK_backend::instance().get_current_extent();
    global_parameters_.screen_size = {static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 0};

    get_Frustum_Planes(global_parameters_.projection_matrix,
                       global_parameters_.view_matrix,
                       global_parameters_.frustum_planes);

    std::array<Eigen::Array4f, 9> &shCoefficients = global_parameters_.shCoefficients;
    // 值是一个差不多结果,不是很精准,因为 输出的 时候只保存了两位小数
    shCoefficients[0] = {1.73, 1.73, 1.73, 0.0f};
    shCoefficients[1] = {-0.05, -0.05, -0.05, 0.0f};
    shCoefficients[2] = {-0.16, -0.16, -0.16, 0.0f};
    shCoefficients[3] = {0.01, 0.01, 0.01, 0.0f};
    shCoefficients[4] = {-0.0, -0.0, -0.0, 0.0f};
    shCoefficients[5] = {0.02, 0.02, 0.02, 0.0f};
    shCoefficients[6] = {0.03, 0.03, 0.03, 0.0f};
    shCoefficients[7] = {-0.01, -0.01, -0.01, 0.0f};
    shCoefficients[8] = {0.01, 0.01, 0.01, 0.0f};

    shCoefficients[0] *= 0.282095f;
    // 对应 l=1
    shCoefficients[1] *= -0.488603f;
    shCoefficients[2] *= 0.488603f;
    shCoefficients[3] *= -0.488603f;

    // 对应 l=2
    shCoefficients[4] *= 1.092548f;
    shCoefficients[5] *= -1.092548f;
    shCoefficients[6] *= 0.315392f;
    shCoefficients[7] *= -1.092548f;
    shCoefficients[8] *= 0.546274f;


    set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                         "global_parameters", global_parameters_);
    set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                         "global_offscreen", offscreen);
    set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                         "global_SSAO", SSAO);
    set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                         "global_depth", depth);


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
    return global_descriptor_sets_[global_descriptor_sets_index];
}

const VKR_image_ptr &Engine::get_current_swap_chain_image() const {
    return get_swap_chain_images().at(get_imageIndex());
}


void Engine::submit_render_queue(uint64_t time_line) {
    // Submit to graphics queue
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
    auto cb = get_current_command_buffer();

    // uint32_t wait_semaphore_len = submit_task->wait_semaphore == VK_NULL_HANDLE ? 0 : 1;
    uint32_t signal_semaphore_len    = 2;
    VkSemaphore signal_semaphores[2] = {
        vk_timeline_semaphore_,
        get_can_render_to_image_semaphores()[get_imageIndex()]
    };
    uint64_t signal_semaphore_values[2] = {time_line, 0};

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_submit_info = {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        signal_semaphore_len,
        signal_semaphore_values
    };

    Command_submit_manager::command_buffer_submit(1, &cb,
                                                  get_current_fences(),
                                                  &timeline_semaphore_submit_info,
                                                  1,
                                                  &get_current_presentSemaphores(),
                                                  &waitStages,
                                                  2,
                                                  signal_semaphores);
}


void Engine::copy_image_to_screen() {
    frameIndex = (frameIndex + 1) % maxFramesInFlight;
    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
        .swapchainCount     = 1,
        .pSwapchains        = &VK_backend::instance().get_swap_chain(),
        .pImageIndices      = &imageIndex
    }; {
        const auto result = Command_submit_manager::command_copy_image_to_screen(presentInfo);
        if (result == VK_SUCCESS) {
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
                   VK_backend::instance().is_frame_buffer_resize()) {
            recreate_swap_chain();
            destroy_and_recreate_fence_and_semaphore();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cout << "failed to acquire swap chain image!" << std::endl;
        }
    }
}


void Engine::get_image_to_render() {
    // forces the CPU to stop and wait until the GPU has finished executing a specific batch of commands
    VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(VK_backend::instance().get_device(), 1, &get_current_fences(), true,
                                 UINT64_MAX));
    VK_CHECK_RESULT_NOT_EXIT(vkResetFences(VK_backend::instance().get_device(), 1, &get_current_fences()));
    auto result = vkAcquireNextImageKHR(VK_backend::instance().get_device(),
                                        VK_backend::instance().get_swap_chain(),
                                        UINT64_MAX,
                                        get_current_presentSemaphores(),
                                        VK_NULL_HANDLE,
                                        &imageIndex); // 其实是在这里执行了 ++ 的工作
    if (result == VK_SUCCESS) {
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
               VK_backend::instance().is_frame_buffer_resize()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        recreate_swap_chain();
        destroy_and_recreate_fence_and_semaphore();
        VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(VK_backend::instance().get_device(), 1, &get_current_fences(), true
                                    ,
                                     UINT64_MAX));
        VK_CHECK_RESULT_NOT_EXIT(vkResetFences(VK_backend::instance().get_device(), 1, &get_current_fences()));
        result = vkAcquireNextImageKHR(VK_backend::instance().get_device(),
                                       VK_backend::instance().get_swap_chain(),
                                       UINT64_MAX,
                                       get_current_presentSemaphores(),
                                       VK_NULL_HANDLE,
                                       &imageIndex);
        if (result == VK_SUCCESS) {
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // 有时候成功，有时候不能一次成功，不知道为什么
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "failed to acquire swap chain image!" << std::endl;
    }
}


void Engine::create_timeline_Semaphores() {
    VkSemaphoreTypeCreateInfo vk_semaphore_type_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, nullptr, VK_SEMAPHORE_TYPE_TIMELINE, 0
    };
    VkSemaphoreCreateInfo vk_semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &vk_semaphore_type_create_info, 0
    };
    vkCreateSemaphore(VK_backend::instance().get_device(), &vk_semaphore_create_info, nullptr, &vk_timeline_semaphore_);
}

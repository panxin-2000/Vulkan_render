//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include "vulkan_code/sets_and_bindings_layout.h"
#include "shader_component.h"
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
    create_render_image();
    create_command_pool();
    create_command_buffer();
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
    create_timeline_Semaphores();
    descriptor_pools.resize(1,VK_NULL_HANDLE);
    descriptor_pools.at(0) = init_current_descriptor_pool();

    VKR_shader_paths shader_paths{
        "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.vert.spv",
        "/Users/panxin/CLionProjects/hello_mac/render/shader/vulkan_different_color.frag.spv",
        "", ""
    };
    shader_data VKR_shader_init(VKR_shader_paths &shader_paths);
    shader_date = VKR_shader_init(shader_paths);


    bindless_descriptor_sets_ = allocate_bindless_descriptor_sets("");
    global_descriptor_sets_   = allocate_global_descriptor_sets("");
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
    for (auto descriptor_pool: descriptor_pools) {
        if (descriptor_pool != VK_NULL_HANDLE)
            destroy_descriptorPool(descriptor_pool);
    }
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

std::vector<DescriptorSet_ptr> Engine::allocate_global_descriptor_sets(const std::string &one_binding_name) {
    auto &handle    = VK_backend::instance();
    auto sets_flags = create_descriptor_sets_flags(handle,
                                                   shader_date->global_sets_bindings);
    auto bindless_descriptor_sets = allocate_descriptor_sets(get_descriptor_pool(),
                                                             shader_date->global_descriptor_sets_layout,
                                                             sets_flags);
    // 这里申请完 descriptor_sets 了
    // 那么之后需要上传参数了
    // 那么应该就算搞定了
    return bindless_descriptor_sets;
}

void Engine::update_global_parameter() {
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_projection_4x4", projection_matrix);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_inv_projection_4x4", inv_projection_matrix);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_view_4x4", view_matrix);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_ins_view_4x4", inv_view_matrix);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_inv_VP", invVP);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_world_view_Pos", world_camera_pos);
    set_render_parameter(shader_date->global_sets_bindings, update_global_descriptor_sets,
                         "global_world_light_Pos", world_light_pos);
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


std::vector<DescriptorSet_ptr> Engine::allocate_bindless_descriptor_sets(const std::string &one_binding_name) {
    auto &handle    = VK_backend::instance();
    auto sets_flags = create_descriptor_sets_flags(handle,
                                                   shader_date->bindless_sets_bindings);
    auto bindless_descriptor_sets = allocate_descriptor_sets(get_descriptor_pool(), shader_date->bindless_set_layout,
                                                             sets_flags);

    return bindless_descriptor_sets;
}


Proxy_descriptor_sets Engine::get_bindless_descriptor_set(const uint index) {
    return bindless_descriptor_sets_;
}

Proxy_descriptor_sets Engine::get_global_descriptor_set(const uint index) {
    return global_descriptor_sets_;
}

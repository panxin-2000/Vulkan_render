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
    descriptor_pools.resize(1,VK_NULL_HANDLE);
    descriptor_pools.at(0) = init_current_descriptor_pool();

    VKR_shader_paths shader_paths{
        "/Users/panxin/CLionProjects/hello_mac/render/shader/Phong.vert.spv",
        "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
        "", ""
    };
    shader_data VKR_shader_init(VKR_shader_paths &shader_paths);
    shader_date               = VKR_shader_init(shader_paths);
    bindless_descriptor_sets_ = allocate_bindless_descriptor_sets("");
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


    global_descriptor_sets_ = allocate_global_descriptor_sets("");
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

void Engine::update_global_pbr_parameter(
    std::map<std::string, Update_descriptor_binding> &update_global_descriptor_sets) {
    auto size = pbr_manager_.size() * sizeof(PBR_component);
    if (size > 0) {
        auto src = pbr_manager_.data();
#define ALIGN_1024(size) (((size) + 1023) & ~1023)

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

void Engine::update_global_parameter() {
    global_descriptor_sets_ = allocate_global_descriptor_sets("");
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;
    const auto extent              = VK_backend::instance().get_current_extent();
    global_parameters_.screen_size = {static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 0};
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

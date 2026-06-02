//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include "sets_and_bindings_layout.h"
#include "shader_component.h"
#include "vulkan_backend.h"


void Engine::get_query_results() {
    const auto &backend = VK_backend::get();
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
    const auto &backend = VK_backend::get();
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
    const auto &backend = VK_backend::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyQueryPool(backend.get_device(), query_pools[i], nullptr);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_command_buffer() {
    const auto &backend = VK_backend::get();

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
    const auto &backend = VK_backend::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkFreeCommandBuffers(backend.get_device(), get_command_pool(), 1, &command_buffers_[i]);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_fences() {
    const auto &backend = VK_backend::get();
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(backend.get_device(), &fenceCI, nullptr, &fences_[i]));
    }
}

void Engine::destroy_fences() {
    const auto &backend = VK_backend::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroyFence(backend.get_device(), fences_[i], nullptr); //  这里还需要
        fences_[i] = VK_NULL_HANDLE;
    }
}

void Engine::create_present_Semaphores() {
    const auto &backend = VK_backend::get();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(backend.get_device(), &semaphoreCI,
                                     nullptr, &present_semaphores_[i]));
    }
}

void Engine::destroy_present_Semaphores() {
    const auto &backend = VK_backend::get();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroySemaphore(backend.get_device(), present_semaphores_[i], nullptr); //
        present_semaphores_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_renderSemaphores() {
    const auto &backend = VK_backend::get();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    render_to_image_semaphores_.resize(backend.get_swap_chain_images().size());
    LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
    for (auto &semaphore: render_to_image_semaphores_) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(backend.get_device(), &semaphoreCI, nullptr, &semaphore));
    }
}

void Engine::destroy_renderSemaphores() {
    const auto &backend = VK_backend::get();
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

void Engine::engine_destroy() {
    const auto &backend = VK_backend::get();
    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(backend.get_device()));
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
    const auto &backend = VK_backend::get();
    for (auto command_pool: command_pools_) {
        if (command_pool != VK_NULL_HANDLE)
            vkDestroyCommandPool(backend.get_device(), command_pool, nullptr);
    }
}

void Engine::create_command_pool() {
    // Command pool
    const auto &backend = VK_backend::get();
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
    auto &handle    = VK_backend::get();
    auto sets_flags = create_descriptor_sets_flags(handle,
                                                   shader_date->global_sets_bindings);
    auto bindless_descriptor_sets = allocate_descriptor_sets(
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
    auto bindless_descriptor_sets = VK_backend::get().engine_.get_bindless_descriptor_set();
    auto global_descriptor_sets   = VK_backend::get().engine_.get_global_descriptor_set();
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
    auto &handle    = VK_backend::get();
    auto sets_flags = create_descriptor_sets_flags(handle,
                                                   shader_date->bindless_sets_bindings);
    auto bindless_descriptor_sets = allocate_descriptor_sets(shader_date->bindless_set_layout,
                                                             sets_flags);

    return bindless_descriptor_sets;
}


Proxy_descriptor_sets Engine::get_bindless_descriptor_set(const uint index) {
    return bindless_descriptor_sets_;
}

Proxy_descriptor_sets Engine::get_global_descriptor_set(const uint index) {
    return global_descriptor_sets_;
}

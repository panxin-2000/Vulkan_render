//
// Created by 潘鑫 on 2026/7/26.
//

#ifndef HELLO_MAC_DESCRIPTOR_POOL_MANAGER_H
#define HELLO_MAC_DESCRIPTOR_POOL_MANAGER_H
#include "descriptor_pool.h"
#include "sets_and_bindings_layout.h"
#include "shader_resolve.h"
#include "vulkan_backend.h"


class Descriptor_pool_manager {
private:
    std::vector<VkDescriptorPool> descriptor_pools = {};
    VkDescriptorPool bindless_descriptor_pools     = VK_NULL_HANDLE;

    std::shared_ptr<vk_shader_data> shader_date_;

public:
    void create() {
        descriptor_pools.resize(1,VK_NULL_HANDLE);
        descriptor_pools.at(0) = init_current_descriptor_pool();
    }

    bool set_shader_data(const std::shared_ptr<vk_shader_data> &shader_date) {
        shader_date_ = shader_date;
        return true;
    }

    [[nodiscard]] VkDescriptorPool get_descriptor_pool_for_alloc() const {
        return descriptor_pools.at(descriptor_pools.size() - 1);
    }

    void allocate_descriptor_pool() {
        const auto size = descriptor_pools.size();
        descriptor_pools.resize(size + 1,VK_NULL_HANDLE);
        descriptor_pools.at(size) = init_current_descriptor_pool();
    }

    std::vector<DescriptorSet_ptr> allocate_bindless_descriptor_sets(
        const sets_map &bindless_sets_bindings,
        const std::vector<VkDescriptorSetLayout> &bindless_set_layout,
        uint64_t timeline = 0) {
        bindless_descriptor_pools = descriptor_pools.at(0); // todo: 需要添加申请的函数  // 或者说添加 初始化 时的参数
        auto &handle              = VK_backend::instance();
        auto sets_flags           = create_descriptor_sets_flags(handle, bindless_sets_bindings);
        auto result               = allocate_descriptor_sets(bindless_descriptor_pools, bindless_set_layout,
                                               sets_flags, timeline);

        return result;
    }


    std::vector<DescriptorSet_ptr> allocate_global_descriptor_sets(
        const sets_map &global_sets_bindings,
        const std::vector<VkDescriptorSetLayout> &global_set_layout,
        uint64_t timeline = 0) {
        bindless_descriptor_pools = descriptor_pools.at(0); // todo: 需要添加申请的函数  // 或者说添加 初始化 时的参数
        auto &handle              = VK_backend::instance();
        auto sets_flags           = create_descriptor_sets_flags(handle, global_sets_bindings);
        auto result               = allocate_descriptor_sets(bindless_descriptor_pools, global_set_layout,
                                               sets_flags, timeline);

        return result;
    }

    void clean_shader_data() {
        if (shader_date_ != nullptr) {
            shader_date_ = nullptr;
        }
    }

    void destroy() {
        for (auto descriptor_pool: descriptor_pools) {
            if (descriptor_pool != VK_NULL_HANDLE)
                destroy_descriptorPool(descriptor_pool);
        }
    }
};


#endif //HELLO_MAC_DESCRIPTOR_POOL_MANAGER_H

//
// Created by 潘鑫 on 2026/7/22.
//

#ifndef HELLO_MAC_VULKAN_EXECUTE_COMMAND_H
#define HELLO_MAC_VULKAN_EXECUTE_COMMAND_H

#include <map>
#include <utility>
#include <vk_mem_alloc.h>
#include "APP_utility_mixins.h"


class temp_command_execute {
    VkCommandPool pool            = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

public:
    temp_command_execute();

    void add_execute_function(const std::function<void(VkCommandBuffer commandBuffer)> &callback) const;

    ~temp_command_execute();
};


#endif //HELLO_MAC_VULKAN_EXECUTE_COMMAND_H

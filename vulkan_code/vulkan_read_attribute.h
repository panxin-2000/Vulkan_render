//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_READ_ATTRIBUTE_H
#define HELLO_MAC_VULKAN_READ_ATTRIBUTE_H
#include "vulkan_global_macro.h"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

std::vector<std::string> get_all_instance_extensions(void);

std::vector<VkPhysicalDevice> get_all_physical_devices(const VkInstance &instance);

std::vector<VkExtensionProperties> get_all_physical_extensions(const VkPhysicalDevice &device);

std::vector<VkQueueFamilyProperties> get_queue_family_properties(const VkPhysicalDevice &device);

VkPhysicalDeviceMemoryProperties get_vulkan_memory(const VkPhysicalDevice &device);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

#endif //HELLO_MAC_VULKAN_READ_ATTRIBUTE_H

//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_VULKAN_UTILITY_H
#define HELLO_MAC_VULKAN_UTILITY_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/vulkan.h"
#include <vector>

#include "vulkan_global_macro.h"


void add_platform_need_instance_extensions(VkInstanceCreateInfo &instanceCreateInfo,
                                           std::vector<const char *> &instanceExtensions);


bool check_have_present_support(const VkPhysicalDevice &device,
                                VkQueueFamilyProperties &queueFamily,
                                const int queueFamilyIndex,
                                const VkSurfaceKHR &surface);


bool check_have_queue_graphics(const VkQueueFamilyProperties &queueFamily);

bool check_have_queue_compute(const VkQueueFamilyProperties &queueFamily);

bool check_have_queue_transfer(const VkQueueFamilyProperties &queueFamily);

VkFormat findDepthFormat(const VkPhysicalDevice &physicalDevice);

VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                             VkFormatFeatureFlags features, const VkPhysicalDevice &physicalDevice);

int32_t get_queue_family_index(const VkPhysicalDevice &physical_device, VkSurfaceKHR surface_);
#endif //HELLO_MAC_VULKAN_UTILITY_H

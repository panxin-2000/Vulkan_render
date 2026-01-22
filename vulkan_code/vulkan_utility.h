//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_VULKAN_UTILITY_H
#define HELLO_MAC_VULKAN_UTILITY_H
#include <vector>

#include "vulkan_global_macro.h"


std::vector<std::string> get_instance_extensions(void);

void add_validationLayers(VkInstanceCreateInfo &instanceCreateInfo, VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo,
                          std::vector<const char *> &instanceExtensions);

void add_platform_need_instance_extensions(VkInstanceCreateInfo &instanceCreateInfo,
                                           std::vector<const char *> &instanceExtensions);

std::vector<VkPhysicalDevice> get_all_physical_devices(const VkInstance &instance);

std::vector<VkQueueFamilyProperties> get_queue_family_properties(const VkPhysicalDevice &device);

bool check_have_present_support(const VkPhysicalDevice &device,
                                VkQueueFamilyProperties &queueFamily,
                                const int queueFamilyIndex,
                                const VkSurfaceKHR &surface);


bool check_have_queue_graphics(const VkQueueFamilyProperties &queueFamily);

bool check_have_queue_compute(const VkQueueFamilyProperties &queueFamily);

bool check_have_queue_transfer(const VkQueueFamilyProperties &queueFamily);
#endif //HELLO_MAC_VULKAN_UTILITY_H

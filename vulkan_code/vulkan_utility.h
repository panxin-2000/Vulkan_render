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
#endif //HELLO_MAC_VULKAN_UTILITY_H

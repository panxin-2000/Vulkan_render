//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_VALIDATION_LAYER_H
#define HELLO_MAC_VULKAN_VALIDATION_LAYER_H

#include <volk.h>
#include <vector>

void add_instance_validation_layers(VkInstanceCreateInfo &instanceCreateInfo,
                                    VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo,
                                    std::vector<const char *> &instanceExtensions);


void add_device_validation_layers(VkDeviceCreateInfo &createInfo);


#endif //HELLO_MAC_VULKAN_VALIDATION_LAYER_H

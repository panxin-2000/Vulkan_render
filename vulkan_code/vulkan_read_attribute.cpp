//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_read_attribute.h"

std::vector<std::string> get_all_instance_extensions() {
    std::vector<std::string> supportedInstanceExtensions;
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
            for (VkExtensionProperties &extension: extensions) {
                supportedInstanceExtensions.emplace_back(extension.extensionName);
            }
        }
    }
    return supportedInstanceExtensions;
}


std::vector<VkExtensionProperties> get_all_physical_extensions(const VkPhysicalDevice &device) {
    uint32_t properties_count;
    std::vector<VkExtensionProperties> properties;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &properties_count, nullptr);
    properties.resize(properties_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &properties_count,
                                         properties.data());
    return properties;
}


std::vector<VkPhysicalDevice> get_all_physical_devices(const VkInstance &instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    return devices;
}

std::vector<VkQueueFamilyProperties> get_queue_family_properties(const VkPhysicalDevice &device) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    return queueFamilies;
}

VkPhysicalDeviceMemoryProperties get_vulkan_memory(const VkPhysicalDevice &device) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
    return memoryProperties;
}

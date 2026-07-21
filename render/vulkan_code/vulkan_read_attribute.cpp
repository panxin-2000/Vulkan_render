//
// Created by 潘鑫 on 2026/1/23.
//

#include "vulkan_read_attribute.h"

#include <iostream>
#include <SDL3/SDL_video.h>

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


/**
 * vkGetPhysicalDeviceSurfaceCapabilitiesKHR
 * vkGetPhysicalDeviceSurfaceFormatsKHR
 * vkGetPhysicalDeviceSurfacePresentModesKHR
 *
 * @param device
 * @param surface
 * @return
 */
SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &device, const VkSurfaceKHR &surface) {
    SwapChainSupportDetails details;
    // details.capabilities 物理设备表面功能
    VkSurfaceCapabilitiesKHR temp;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &temp);
    std::cout << "min Image Count : " << temp.minImageCount << std::endl;
    std::cout << "max Image Count : " << temp.maxImageCount << std::endl;
    std::cout << "current Extent x : " << temp.currentExtent.width << std::endl;
    std::cout << "current Extent y : " << temp.currentExtent.height << std::endl;
    std::cout << "min Image Extent x : " << temp.minImageExtent.width << std::endl;
    std::cout << "min Image Extent y : " << temp.minImageExtent.height << std::endl;
    std::cout << "max Image Extent x : " << temp.maxImageExtent.width << std::endl;
    std::cout << "max Image Extent y : " << temp.maxImageExtent.height << std::endl;
    std::cout << "max Image Array Layers : " << temp.maxImageArrayLayers << std::endl;
    details.capabilities = temp;


    return details;
}

VkExtent2D get_swap_image_rational_extent(const VkPhysicalDevice &device, const VkSurfaceKHR &surface,
                                          SDL_Window *window) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        SDL_GetWindowSizeInPixels(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

uint32_t get_rational_image_count(const VkPhysicalDevice &device, const VkSurfaceKHR &surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
    uint32_t imageCount = get_max_frames_in_flight(); // 这里的值其实不能写死，应该由双缓冲函数三缓冲决定
    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    return imageCount;
}

VkSurfaceFormatKHR choose_swap_surface_format(const VkPhysicalDevice &device, const VkSurfaceKHR &surface) {
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
    }
    // VK_FORMAT_B8G8R8A8_SRGB 和 how to vulkan 2026 重的值是不一致的
    for (const auto &availableFormat: formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return formats[0];
}

VkPresentModeKHR choose_swap_present_mode(const VkPhysicalDevice &device, const VkSurfaceKHR &surface) {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
    }
    for (const auto &availablePresentMode: presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

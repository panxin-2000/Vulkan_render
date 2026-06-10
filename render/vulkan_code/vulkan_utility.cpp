//
// Created by 潘鑫 on 2026/1/22.
//

#include <iostream>
#include <vector>
#include <volk.h>
#include "vulkan_utility.h"

#include <fstream>

#include "vulkan_read_attribute.h"


/**
 *  VK_EXT_metal_surface
 *  VK_KHR_portability_enumeration
 *  instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
 * @param instanceCreateInfo
 * @param instanceExtensions
 */
void add_platform_need_instance_extensions(VkInstanceCreateInfo &instanceCreateInfo,
                                           std::vector<const char *> &instanceExtensions) {
#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK) || defined(VK_USE_PLATFORM_METAL_EXT)) && defined(VK_KHR_portability_enumeration)
    {
        auto supportedInstanceExtensions = get_all_instance_extensions();
        // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_enumeration is defined and supported by the instance, enable the extension and the flag
        if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
        instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    }
#endif
}


bool check_have_present_support(const VkPhysicalDevice &device,
                                VkQueueFamilyProperties &queueFamily,
                                const int queueFamilyIndex,
                                const VkSurfaceKHR &surface) {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, queueFamilyIndex, surface, &presentSupport);
    if (presentSupport)
        return true;
    return false;
}

bool check_have_queue_graphics(const VkQueueFamilyProperties &queueFamily) {
    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        return true;
    return false;
}

bool check_have_queue_compute(const VkQueueFamilyProperties &queueFamily) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        return true;
    return false;
}

bool check_have_queue_transfer(const VkQueueFamilyProperties &queueFamily) {
    if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        return true;
    return false;
}


VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                             VkFormatFeatureFlags features, const VkPhysicalDevice &physicalDevice) {
    for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }
    throw std::runtime_error("failed to find supported format!");
}

VkFormat findDepthFormat(const VkPhysicalDevice &physicalDevice) {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physicalDevice
    );
}


static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

int32_t get_queue_family_index(const VkPhysicalDevice &physical_device, VkSurfaceKHR surface_) {
    auto family_properties = get_queue_family_properties(physical_device);
    int queueFamilyIndex = 0;
    for (auto family_property: family_properties) {
        bool temp_1 = check_have_queue_compute(family_property);
        bool temp_2 = check_have_queue_graphics(family_property);
        bool temp_3 = check_have_queue_graphics(family_property);
        bool temp_4 = check_have_present_support(physical_device, family_property, queueFamilyIndex, surface_);
        if (temp_1 && temp_2 && temp_3 && temp_4) {
            return queueFamilyIndex;
        }
        queueFamilyIndex++;
    }
    return -1;
}

void createCommandPool(const VkPhysicalDevice &physical_device, VkDevice device, VkSurfaceKHR surface,
                       VkCommandPool &commandPool) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = get_queue_family_index(physical_device, surface);

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

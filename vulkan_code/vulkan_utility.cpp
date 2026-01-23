//
// Created by 潘鑫 on 2026/1/22.
//

#include <iostream>
#include <vector>
#include <volk.h>
#include "vulkan_utility.h"



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

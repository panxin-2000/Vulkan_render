//
// Created by 潘鑫 on 2026/1/22.
//

#include <iostream>
#include <vector>
#include <volk.h>
#include "vulkan_utility.h"

std::vector<std::string> get_instance_extensions(void) {
    std::vector<std::string> supportedInstanceExtensions;
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0) {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
            for (VkExtensionProperties &extension: extensions) {
                supportedInstanceExtensions.push_back(extension.extensionName);
            }
        }
    }
    return supportedInstanceExtensions;
}

static const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void add_validationLayers(VkInstanceCreateInfo &instanceCreateInfo, VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo,
                          std::vector<const char *> &instanceExtensions) {
    {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        auto supportedInstanceExtensions = get_instance_extensions();
        if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                      VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end()) {
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
            instanceCreateInfo.enabledLayerCount = validationLayers.size();
            instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
        }
    }
}


void add_device_validation_layers(VkDeviceCreateInfo &createInfo) {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
}

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
        auto supportedInstanceExtensions = get_instance_extensions();
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

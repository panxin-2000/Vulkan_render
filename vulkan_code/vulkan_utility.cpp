//
// Created by 潘鑫 on 2026/1/22.
//

#include <iostream>
#include <vector>
#include <volk.h>

static std::vector<std::string> get_instance_extensions(void) {
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
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
        }
    }
}

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

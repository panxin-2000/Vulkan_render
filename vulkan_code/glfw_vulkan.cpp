//
// Created by 潘鑫 on 2026/1/22.
//

// 下面这个只能在一个 cpp 文件中定义
#ifdef ENGINE_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif
#include "glfw_vulkan.h"


vulkan_create_screen::vulkan_create_screen() {
#ifdef ENGINE_USE_VOLK
    if (volkInitialize() != VK_SUCCESS) {
        return;
    }
#endif
    createInstance();

    createSurface();

    choose_one_physical_device(physical_device_);

    //
}

vulkan_create_screen::~vulkan_create_screen() {
    volkFinalize();
}


void vulkan_create_screen::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = ApplicationName.c_str();
    appInfo.applicationVersion = applicationVersion;
    appInfo.pEngineName = EngineName.c_str();
    appInfo.engineVersion = engineVersion;
    appInfo.apiVersion = apiVersion;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    add_platform_need_instance_extensions(instanceCreateInfo, instanceExtensions);

#ifndef NDEBUG //  cmake_build_type 在build 模式下不产生 NDEBUG 宏
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    add_validationLayers(instanceCreateInfo, debugCreateInfo, instanceExtensions);
#else

#endif
    instanceExtensions.push_back("VK_KHR_surface");
    instanceExtensions.push_back("VK_KHR_get_physical_device_properties2");

    instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (instanceExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    } else {
        instanceCreateInfo.enabledExtensionCount = 0;
        instanceCreateInfo.ppEnabledExtensionNames = nullptr;
    }
    auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance_);
    if (err != VK_SUCCESS) {
    } else {
#ifdef ENGINE_USE_VOLK
        volkLoadInstance(instance_);
#endif
    }
}

void vulkan_create_screen::createSurface() {
    glfwInit();
    if (GLFW_TRUE == glfwVulkanSupported()) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        auto result = glfwCreateWindowSurface(instance_, window_, Allocator, &surface_);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}

bool vulkan_create_screen::choose_one_physical_device(VkPhysicalDevice &PhysicalDevice) {
    auto physical_devices = get_all_physical_devices(instance_);
    for (auto physical_device: physical_devices) {
        auto family_properties = get_queue_family_properties(physical_device);
        int queueFamilyIndex = 0;
        for (auto family_property: family_properties) {
            bool temp_1 = check_have_queue_compute(family_property);
            bool temp_2 = check_have_queue_graphics(family_property);
            bool temp_3 = check_have_queue_graphics(family_property);
            bool temp_4 = check_have_present_support(physical_device, family_property, queueFamilyIndex, surface_);
            if (temp_1 && temp_2 && temp_3 && temp_4) {
                PhysicalDevice = physical_device;
                return true;
            }
            queueFamilyIndex++;
        }
    }
    return false;
}

uint32_t get_queue_family_index(const VkPhysicalDevice &physical_device, VkSurfaceKHR surface_) {
    auto family_properties = get_queue_family_properties(physical_device);
    int queueFamilyIndex = 0;
    for (auto family_property: family_properties) {
        bool temp_1 = check_have_queue_compute(family_property);
        bool temp_2 = check_have_queue_graphics(family_property);
        bool temp_3 = check_have_queue_graphics(family_property);
        bool temp_4 = check_have_present_support(physical_device, family_property, queueFamilyIndex, surface_);
        if (temp_1 && temp_2 && temp_3 && temp_4) {
            //
        }
        return queueFamilyIndex;
        queueFamilyIndex++;
    }
    return 0;
}


// void vulkan_create_screen::createDevice() {
//     std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
//     const float defaultQueuePriority(0.0f);
//     // Graphics queue
//     if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
//         // getQueueFamilyIndex 这个函数很好
//         queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
//         VkDeviceQueueCreateInfo queueInfo{};
//         queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//         queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
//         queueInfo.queueCount = 1;
//         queueInfo.pQueuePriorities = &defaultQueuePriority;
//         queueCreateInfos.push_back(queueInfo);
//     } else {
//         queueFamilyIndices.graphics = 0;
//     }
//     // Dedicated compute queue
//     if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
//         queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
//         if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
//             // If compute family index differs, we need an additional queue create info for the compute queue
//             VkDeviceQueueCreateInfo queueInfo{};
//             queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//             queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
//             queueInfo.queueCount = 1;
//             queueInfo.pQueuePriorities = &defaultQueuePriority;
//             queueCreateInfos.push_back(queueInfo);
//         }
//     } else {
//         // Else we use the same queue
//         queueFamilyIndices.compute = queueFamilyIndices.graphics;
//     }
//
//
//     // Dedicated transfer queue
//     if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
//         queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
//         if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (
//                 queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
//             // If transfer family index differs, we need an additional queue create info for the transfer queue
//             VkDeviceQueueCreateInfo queueInfo{};
//             queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//             queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
//             queueInfo.queueCount = 1;
//             queueInfo.pQueuePriorities = &defaultQueuePriority;
//             queueCreateInfos.push_back(queueInfo);
//         }
//     } else {
//         // Else we use the same queue
//         queueFamilyIndices.transfer = queueFamilyIndices.graphics;
//     }
//
//
//     VkDeviceCreateInfo createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//     createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//     createInfo.pQueueCreateInfos = queueCreateInfos.data();
//     createInfo.pEnabledFeatures = &deviceFeatures;
//     createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//     createInfo.ppEnabledExtensionNames = deviceExtensions.data();
//     if (enableValidationLayers) {
//         createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
//         createInfo.ppEnabledLayerNames = validationLayers.data();
//     } else {
//         createInfo.enabledLayerCount = 0;
//     }
//     if (vkCreateDevice(physical_device_, &createInfo, Allocator, &device_) != VK_SUCCESS) {
//     }
//     vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue);
//     vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue);
// }

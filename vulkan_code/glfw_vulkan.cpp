//
// Created by 潘鑫 on 2026/1/22.
//

// 下面这个只能在一个 cpp 文件中定义

#include "glfw_vulkan.h"


vulkan_create_screen::vulkan_create_screen() {
    glfwInit();

    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // window_ = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    // glfwSetWindowUserPointer(window_, this);

    createInstance();
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    }

    createSurface();

    choose_one_physical_device(physical_device_);
}

vulkan_create_screen::~vulkan_create_screen() {
}


void vulkan_create_screen::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ApplicationName.c_str()";
    appInfo.applicationVersion = 123;
    appInfo.pEngineName = "EngineName.c_str()";
    appInfo.engineVersion = 12;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pNext = nullptr;

    std::vector<const char *> instanceExtensions;


    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceExtensions.push_back("VK_KHR_surface");
    instanceExtensions.push_back("VK_EXT_metal_surface");
    instanceExtensions.push_back("VK_KHR_get_physical_device_properties2");
    instanceExtensions.push_back("VK_KHR_portability_enumeration");

    instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

    auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance_); {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    } {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    } {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    }
}

void vulkan_create_screen::createSurface() {
    if (auto value = glfwCreateWindowSurface(instance_, window_, Allocator, &surface_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
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
            }

            queueFamilyIndex++;
        }
    }
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

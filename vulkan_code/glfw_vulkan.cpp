//
// Created by 潘鑫 on 2026/1/22.
//

// 下面这个只能在一个 cpp 文件中定义
#ifdef ENGINE_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif
#include "glfw_vulkan.h"

#include "vulkan_image.h"


vulkan_create_screen::vulkan_create_screen() {
#ifdef ENGINE_USE_VOLK
    if (volkInitialize() != VK_SUCCESS) {
        return;
    }
#endif
    createInstance();

    createSurface();

    choose_one_physical_device(physical_device_);

    createDevice();
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
    add_instance_validation_layers(instanceCreateInfo, debugCreateInfo, instanceExtensions);
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
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // 允许屏幕的缩放
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


void vulkan_create_screen::createDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    const float defaultQueuePriority(0.0f);
    // Graphics queue
    if (1) {
        // getQueueFamilyIndex 这个函数很好
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = get_queue_family_index(physical_device_, surface_);
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    } else {
    }
    // Dedicated compute queue
    if (1) {
        // If compute family index differs, we need an additional queue create info for the compute queue
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = get_queue_family_index(physical_device_, surface_);;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    } else {
        // Else we use the same queue
    }
    // Dedicated transfer queue
    if (1) {
        // If transfer family index differs, we need an additional queue create info for the transfer queue
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = get_queue_family_index(physical_device_, surface_);
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    } else {
        // Else we use the same queue
    }
    std::vector<const char *> device_extensions;
    device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    device_extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);


    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE; // 仅开启各向异性过滤
    // 还有很多的特征，按照需要添加。

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    createInfo.ppEnabledExtensionNames = device_extensions.data();
#ifndef NDEBUG //  cmake_build_type 在build 模式下不产生 NDEBUG 宏
    add_device_validation_layers(createInfo);
#else
    createInfo.enabledLayerCount = 0;
#endif
    if (vkCreateDevice(physical_device_, &createInfo, Allocator, &device_) != VK_SUCCESS) {
    }
    vkGetDeviceQueue(device_, get_queue_family_index(physical_device_, surface_), 0, &graphicsQueue);
    vkGetDeviceQueue(device_, get_queue_family_index(physical_device_, surface_), 0, &presentQueue);
    vkGetDeviceQueue(device_, get_queue_family_index(physical_device_, surface_), 0, &transferQueue);
    //  graphicsQueue presentQueue transferQueue 大概率是相同的，提交任务时需要加锁
}


VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode: availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

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




VkExtent2D vulkan_create_screen::get_current_extent() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device_, surface_);
    const VkExtent2D extent = chooseSwapExtent(window_, swapChainSupport.capabilities);
    return extent;
}


void vulkan_create_screen::create_swapchain() {
    //querySwapChainSupport 在选择物理设备的时候检查过
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device_, surface_);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(window_, swapChainSupport.capabilities);

    //        swapChainSupport.capabilities.minImageCount  的值在Mac上是2
    uint32_t imageCount = 2;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // QueueFamilyIndices indices = find_Queue_Families(physicalDevice);
    // uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // if (indices.graphicsFamily != indices.presentFamily) {
    // createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    // createInfo.queueFamilyIndexCount = 2;
    // createInfo.pQueueFamilyIndices = queueFamilyIndices;
    // } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    std::vector<VkImage> swapChainImages; // 两个image 由 swapChain_管理，就不放到类中了

    //这里的设置重新设置的操作很细节
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages.data());
    //        swapChainImages这个变量是全局变量，但是在这里才确定了数量和地址
    //        下面还有一个resize，先假定那个resize不会更改地址
    //        实际上是确定不会更改地址的，为什么要在下面再 resize 一遍呢？
    //        因为是不一样的，VkImage 和 VkImageView


    // swapChainImageFormat = surfaceFormat.format; // render pass 的时候还需要使用
    // swapChainExtent = extent;

    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(device_, swapChainImages[i], surfaceFormat.format,
                                                 VK_IMAGE_ASPECT_COLOR_BIT);
    }
}


void vulkan_create_screen::createDepthResources() {
    VkFormat depthFormat = findDepthFormat(physical_device_);
    auto swapChainExtent = get_current_extent();
    createImage(physical_device_, device_, swapChainExtent.width, swapChainExtent.height, depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage,
                depthImageMemory);
    depthImageView = createImageView(device_, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

//
// Created by 潘鑫 on 2026/1/22.
//

// 下面这个只能在一个 cpp 文件中定义
#ifdef ENGINE_USE_VOLK
#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <volk.h>
#endif
#include "vulkan_device_handle.h"

#include "vulkan_image.h"

static inline void chk(VkResult result) {
    if (result != VK_SUCCESS) {
        std::cerr << "Vulkan call returned an error (" << result << ")\n";
        exit(result);
    }
}

static inline void chk(bool result) {
    if (!result) {
        std::cerr << "Call returned an error\n";
        exit(result);
    }
}

VKDevice::VKDevice() {
}

VKDevice::~VKDevice() {
    volkFinalize();
}


void VKDevice::create_instance() {
    if (volkInitialize() != VK_SUCCESS) {
        return;
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = application_name_.c_str();
    appInfo.applicationVersion = application_version_;
    appInfo.pEngineName = engine_name_.c_str();
    appInfo.engineVersion = engine_version_;
    appInfo.apiVersion = api_version_;
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

void VKDevice::create_surface() {
    glfwInit();
    if (GLFW_TRUE == glfwVulkanSupported()) {
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // 允许屏幕的缩放
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(1280, 720, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        auto result = glfwCreateWindowSurface(instance_, window_, Allocator, &surface_);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}

bool VKDevice::choose_one_physical_device() {
    auto physical_devices = get_all_physical_devices(instance_);
    // for (auto physical_device: physical_devices) {
    //     auto family_properties = get_queue_family_properties(physical_device);
    //     int queueFamilyIndex = 0;
    //     for (auto family_property: family_properties) {
    //         bool temp_1 = check_have_queue_compute(family_property);
    //         bool temp_2 = check_have_queue_graphics(family_property);
    //         bool temp_3 = check_have_queue_graphics(family_property);
    //         bool temp_4 = check_have_present_support(physical_device, family_property, queueFamilyIndex, surface_);
    //         if (temp_1 && temp_2 && temp_3 && temp_4) {
    //             PhysicalDevice = physical_device;
    //             return true;
    //         }
    //         queueFamilyIndex++;
    //     }
    // }
    // return false;
    uint32_t deviceIndex{0};
    // if (argc > 1) {
    // deviceIndex = std::stoi(argv[1]);
    // assert(deviceIndex < deviceCount);
    // }
    VkPhysicalDeviceProperties2 deviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
    vkGetPhysicalDeviceProperties2(physical_devices[deviceIndex], &deviceProperties);
    std::cout << "Selected device: " << deviceProperties.properties.deviceName << "\n";
    // Find a queue family for graphics
    uint32_t queueFamilyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[deviceIndex], &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[deviceIndex], &queueFamilyCount, queueFamilies.data());
    for (size_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_ = i;
            break;
        }
    }

    physical_device_ = physical_devices[deviceIndex];
    return true; // how to vulkan 版的内容
}


void VKDevice::create_device() {
    // Logical device
    const float qfpriorities{1.0f};
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = queue_family_, .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };
    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .descriptorIndexing = true,
        .descriptorBindingVariableDescriptorCount = true, .runtimeDescriptorArray = true, .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &enabledVk12Features,
        .synchronization2 = true, .dynamicRendering = true
    };
    std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    const VkPhysicalDeviceFeatures enabledVk10Features{.samplerAnisotropy = VK_TRUE};
    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledVk13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &enabledVk10Features
    };
    chk(vkCreateDevice(physical_device_, &deviceCI, nullptr, &device_));
    vkGetDeviceQueue(device_, queue_family_, 0, &graphics_queue_);
    return; // how to vulkan 的 内容


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
    vkGetDeviceQueue(device_, get_queue_family_index(physical_device_, surface_), 0, &graphics_queue_);
    vkGetDeviceQueue(device_, get_queue_family_index(physical_device_, surface_), 0, &present_queue_);
    vkGetDeviceQueue(device_, get_queue_family_index(physical_device_, surface_), 0, &transfer_queue_);
    //  graphicsQueue presentQueue transferQueue 大概率是相同的，提交任务时需要加锁
}

void VKDevice::create_VMA() {
    // VMA
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorCI{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physical_device_,
        .device = device_, .pVulkanFunctions = &vkFunctions,
        .instance = instance_
    };
    chk(vmaCreateAllocator(&allocatorCI, &allocator_));
}


VkExtent2D VKDevice::get_current_extent() {
    // todo: 这个函数应该是稍微有点重复了的，一会儿转移
    VkSurfaceCapabilitiesKHR surface_caps;
    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps));
    const VkExtent2D extent = get_swap_rational_extent(window_, surface_caps);
    return extent;
}


void VKDevice::create_swap_chain() {
    // Swap chain

    chk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps_));
    const VkExtent2D extent = get_swap_rational_extent(window_, surface_caps_);

    uint32_t imageCount = get_rational_image_Count(surface_caps_);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physical_device_, surface_);

    VkPresentModeKHR presentMode = chooseSwapPresentMode(physical_device_, surface_);

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface_,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    chk(vkCreateSwapchainKHR(device_, &swapchainCI, nullptr, &swap_chain_));
    return; // how to vulkan
    //
    //     .surface = surface_,
    // .minImageCount = imageCount,
    // .imageFormat = surfaceFormat.format,
    // .imageColorSpace = surfaceFormat.colorSpace,
    // .imageExtent = extent,
    // .imageArrayLayers = 1,
    // .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    // .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // todo
    //
    // .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
    // // .preTransform = surface_caps_.currentTransform,
    //
    // .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    // .presentMode = presentMode,
    // .clipped = VK_TRUE,
    // .oldSwapchain = VK_NULL_HANDLE,

    // QueueFamilyIndices indices = find_Queue_Families(physicalDevice);
    // uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    // if (indices.graphicsFamily != indices.presentFamily) {
    // createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    // createInfo.queueFamilyIndexCount = 2;
    // createInfo.pQueueFamilyIndices = queueFamilyIndices;
    // } else {
    // createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // }


    std::vector<VkImage> swapChainImages; // 两个image 由 swapChain_管理，就不放到类中了

    //这里的设置重新设置的操作很细节
    vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, swapChainImages.data());
    //        swapChainImages这个变量是全局变量，但是在这里才确定了数量和地址
    //        下面还有一个resize，先假定那个resize不会更改地址
    //        实际上是确定不会更改地址的，为什么要在下面再 resize 一遍呢？
    //        因为是不一样的，VkImage 和 VkImageView


    // swapChainImageFormat = surfaceFormat.format; // render pass 的时候还需要使用
    // swapChainExtent = extent;

    swap_chain_image_views_.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swap_chain_image_views_[i] = createImageView(device_, swapChainImages[i], surfaceFormat.format,
                                                     VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VKDevice::create_swap_chain_image_view() {
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physical_device_, surface_);
    uint32_t imageCount{0};
    chk(vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, nullptr));
    swap_chain_images_.resize(imageCount);
    chk(vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, swap_chain_images_.data()));
    swap_chain_image_views_.resize(imageCount);
    for (auto i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewCI{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swap_chain_images_[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceFormat.format,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        };
        chk(vkCreateImageView(device_, &viewCI, nullptr, &swap_chain_image_views_[i]));
    }
}

void VKDevice::create_depth_resources() {
    // VkFormat depthFormat = findDepthFormat(physical_device_);
    // auto swapChainExtent = get_current_extent();
    // createImage(physical_device_, device_, swapChainExtent.width, swapChainExtent.height, depthFormat,
    //             VK_IMAGE_TILING_OPTIMAL,
    //             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage,
    //             depthImageMemory);
    // depthImageView = createImageView(device_, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}


void VKDevice::create_depth_image_view() {
    // Depth attachment
    std::vector<VkFormat> depthFormatList{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    for (VkFormat &format: depthFormatList) {
        VkFormatProperties2 formatProperties{.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        vkGetPhysicalDeviceFormatProperties2(physical_device_, format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            depth_format_ = format;
            break;
        }
    }
    assert(depth_format_ != VK_FORMAT_UNDEFINED);
    VkImageCreateInfo depthImageCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format_,
        .extent{
            .width = surface_caps_.currentExtent.width,
            .height = surface_caps_.currentExtent.height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO
    };
    chk(vmaCreateImage(allocator_, &depthImageCI, &allocCI, &depth_image_, &depthImageAllocation, nullptr));
    VkImageViewCreateInfo depthViewCI{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depth_image_,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depth_format_,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    chk(vkCreateImageView(device_, &depthViewCI, nullptr, &depth_image_view_));
}

void VKDevice::destroy() {
    vmaDestroyImage(allocator_, depth_image_, depthImageAllocation);
    vkDestroyImageView(device_, depth_image_view_, nullptr);
    for (auto i = 0; i < swap_chain_image_views_.size(); i++) {
        vkDestroyImageView(device_, swap_chain_image_views_[i], nullptr);
    }


    vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
    vkDestroySurfaceKHR(instance_, surface_, nullptr);

    VmaTotalStatistics stats;
    vmaCalculateStatistics(allocator_, &stats);

    // 获取全局未销毁的分配总数
    uint32_t activeAllocCount = stats.total.statistics.allocationCount;

    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(device_, nullptr);
    vkDestroyInstance(instance_, nullptr);
    glfwDestroyWindow(window_);
    glfwTerminate();
}

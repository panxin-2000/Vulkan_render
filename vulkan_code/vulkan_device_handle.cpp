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
#include "global_singleton.h"

static VK_handle *instance = nullptr;


VK_handle &VK_handle::get() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        instance = new VK_handle();
        assert(instance != nullptr);
        instance->init_device_handle();
    });
    return *instance;
}


VK_handle::~VK_handle() {
    volkFinalize();
}


void VK_handle::create_instance() {
    if (volkInitialize() != VK_SUCCESS) {
        return;
    }
    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = application_name_.c_str();
    appInfo.applicationVersion = application_version_;
    appInfo.pEngineName        = engine_name_.c_str();
    appInfo.engineVersion      = engine_version_;
    appInfo.apiVersion         = api_version_;
    appInfo.pNext              = nullptr;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    add_platform_need_instance_extensions(instanceCreateInfo, instanceExtensions);

#ifndef NDEBUG //  cmake_build_type 在build 模式下不产生 NDEBUG 宏
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    add_instance_validation_layers(instanceCreateInfo, debugCreateInfo, instanceExtensions);
#else

#endif
    instanceExtensions.push_back("VK_KHR_surface");
    instanceExtensions.push_back("VK_KHR_get_physical_device_properties2");

    instanceCreateInfo.enabledExtensionCount   = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (instanceExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount   = instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    } else {
        instanceCreateInfo.enabledExtensionCount   = 0;
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

static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app                = reinterpret_cast<VK_handle *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void VK_handle::create_surface() {
    glfwInit();
    if (GLFW_TRUE == glfwVulkanSupported()) {
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);    // 允许屏幕的缩放
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window_ = glfwCreateWindow(1280, 720, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
        auto result = glfwCreateWindowSurface(instance_, window_, VK_ORIGINAL_Allocator, &surface_);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}

bool VK_handle::choose_one_physical_device() {
    auto physical_devices = get_all_physical_devices(instance_);
    for (auto physical_device: physical_devices) {
        auto family_properties = get_queue_family_properties(physical_device);
        int queueFamilyIndex   = 0;
        for (auto family_property: family_properties) {
            bool temp_1 = check_have_queue_compute(family_property);
            bool temp_2 = check_have_queue_graphics(family_property);
            bool temp_3 = check_have_queue_graphics(family_property);
            bool temp_4 = check_have_present_support(physical_device, family_property, queueFamilyIndex, surface_);
            if (temp_1 && temp_2 && temp_3 && temp_4) {
                physical_device_ = physical_device;
                VkPhysicalDeviceProperties2 deviceProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
                vkGetPhysicalDeviceProperties2(physical_device_, &deviceProperties);
                LOG_INFO(g_log(), "Selected device:  {}!", deviceProperties.properties.deviceName);
                return true;
            }
            queueFamilyIndex++;
        }
    }
}


uint32_t VK_handle::getQueueFamilyIndex(VkQueueFlags queueFlags) const {
    auto queueFamilyProperties = get_queue_family_properties(physical_device_);

    // Dedicated queue for compute
    // Try to find a queue family index that supports compute but not graphics
    if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    // Dedicated queue for transfer
    // Try to find a queue family index that supports transfer but not graphics and compute
    if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
                ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
            return i;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}


void VK_handle::create_device() {
    // Logical device

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    const float defaultQueuePriority(0.0f); // 1.0 表示最高优先级，0.0 表示最低优先级
    VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT;
    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
        queueInfo.queueCount       = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
    } else {
        queueFamilyIndices.graphics = 0;
    }

    // Dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
        queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
            // If compute family index differs, we need an additional queue create info for the compute queue
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
            queueInfo.queueCount       = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    } else {
        // Else we use the same queue
        queueFamilyIndices.compute = queueFamilyIndices.graphics;
    }
    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
        queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (
                queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
            // If transfer family index differs, we need an additional queue create info for the transfer queue
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
            queueInfo.queueCount       = 1;
            queueInfo.pQueuePriorities = &defaultQueuePriority;
            queueCreateInfos.push_back(queueInfo);
        }
    } else {
        // Else we use the same queue
        queueFamilyIndices.transfer = queueFamilyIndices.graphics;
    }


    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType                                        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing                           = true,
        .shaderSampledImageArrayNonUniformIndexing    = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingPartiallyBound              = true,
        .descriptorBindingVariableDescriptorCount     = true,
        .runtimeDescriptorArray                       = true,
        .scalarBlockLayout                            = true,
        .timelineSemaphore                            = true,
        .bufferDeviceAddress                          = true,
    };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &enabledVk12Features,
        .synchronization2 = true, .dynamicRendering                                       = true
    };
    std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

    const VkPhysicalDeviceFeatures enabledVk10Features{
        .samplerAnisotropy = VK_TRUE,
    };
    VkDeviceCreateInfo deviceCI{
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &enabledVk13Features,
        .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos       = queueCreateInfos.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures        = &enabledVk10Features
    };
#ifndef NDEBUG //  cmake_build_type 在build 模式下不产生 NDEBUG 宏
    add_device_validation_layers(deviceCI);
#else
    createInfo.enabledLayerCount = 0;
#endif
    VK_CHECK_RESULT(vkCreateDevice(physical_device_, &deviceCI, nullptr, &device_));
#ifdef ENGINE_USE_VOLK
    volkLoadDevice(device_);
#endif
    // 如果想高效的一步
    // queueFamilyIndices.graphics
    // queueFamilyIndices.compute
    // queueFamilyIndices.transfer
    // 都应该在不同的 index , getQueueFamilyIndex 必然要更改
    vkGetDeviceQueue(device_, queueFamilyIndices.graphics, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, queueFamilyIndices.compute, 0, &compute_queue_);
    vkGetDeviceQueue(device_, queueFamilyIndices.transfer, 0, &transfer_queue_);
    return;
}

void VK_handle::create_VMA() {
    // VMA
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr   = vkGetDeviceProcAddr,
        .vkCreateImage         = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorCI{
        .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physical_device_,
        .device         = device_, .pVulkanFunctions = &vkFunctions,
        .instance       = instance_
    };
    VK_CHECK_RESULT(vmaCreateAllocator(&allocatorCI, &allocator_));
}


void VK_handle::create_swap_chain(VkSwapchainKHR old_swap_chain) {
    // Swap chain
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    const VkExtent2D extent = get_swap_image_rational_extent(physical_device_, surface_, window_);

    uint32_t imageCount = get_rational_image_count(physical_device_, surface_);

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physical_device_, surface_);

    VkPresentModeKHR presentMode = choose_swap_present_mode(physical_device_, surface_);

    VkSwapchainCreateInfoKHR swapchainCI{
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = surface_,
        .minImageCount    = imageCount,
        .imageFormat      = surfaceFormat.format,
        .imageColorSpace  = surfaceFormat.colorSpace,
        .imageExtent      = extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // todo

        // .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .preTransform = capabilities.currentTransform, //

        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode    = presentMode,
        .clipped        = VK_TRUE,
        .oldSwapchain   = old_swap_chain,
    };
    VK_CHECK_RESULT(vkCreateSwapchainKHR(device_, &swapchainCI, nullptr, &swap_chain_));
    return; // how to vulkan
    // VK_ERROR_NATIVE_WINDOW_IN_USE_KHR
    // 确保每个窗口只创建一个 VkSurfaceKHR 对象

    // if (indices.graphicsFamily != indices.presentFamily) {
    // createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    // createInfo.queueFamilyIndexCount = 2;
    // createInfo.pQueueFamilyIndices = queueFamilyIndices;
    // } else {
    // createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // }
}

void VK_handle::create_swap_chain_image_and_view() {
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physical_device_, surface_);
    uint32_t imageCount{0};
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, nullptr));

    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    images.resize(imageCount);
    image_views.resize(imageCount);

    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, images.data()));
    for (auto i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewCI{
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image    = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = surfaceFormat.format,
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        };
        VK_CHECK_RESULT(vkCreateImageView(device_, &viewCI, nullptr, &image_views[i]));
    }
    for (auto i = 0; i < imageCount; i++) {
        swap_chain_images_.emplace_back(images[i],VK_NULL_HANDLE, image_views[i]);
    }
}

/**
 * 虽然不用，暂时先留着
 */
void VK_handle::create_depth_resources() {
    // VkFormat depthFormat = findDepthFormat(physical_device_);
    // auto swapChainExtent = get_current_extent();
    // createImage(physical_device_, device_, swapChainExtent.width, swapChainExtent.height, depthFormat,
    //             VK_IMAGE_TILING_OPTIMAL,
    //             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage,
    //             depthImageMemory);
    // depthImageView = createImageView(device_, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}


VKR_image_ptr VK_handle::create_depth_image_and_view() {
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
    const VkExtent2D extent = get_swap_image_rational_extent(physical_device_, surface_, window_);

    assert(depth_format_ != VK_FORMAT_UNDEFINED);
    VkImageCreateInfo depthImageCI{
        .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format    = depth_format_,
        .extent{
            .width  = extent.width,
            .height = extent.height,
            .depth  = 1
        },
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    const VmaAllocationCreateInfo allocCI{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO
    };
    VkImage depth_image                = VK_NULL_HANDLE;
    VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
    VkImageView depth_image_view       = VK_NULL_HANDLE;

    VK_CHECK_RESULT(vmaCreateImage(allocator_, &depthImageCI, &allocCI, &depth_image, &depthImageAllocation,nullptr));
    const VkImageViewCreateInfo depthViewCI{
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image    = depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = depth_format_,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };
    VK_CHECK_RESULT(vkCreateImageView(device_, &depthViewCI, nullptr, &depth_image_view));
    return {depth_image, depthImageAllocation, depth_image_view};
}

void VK_handle::destroy() { {
        // 基本上是一个整体
        depth_image_->destroy_image();
        for (const auto &image: swap_chain_images_) {
            image->destroy_image();
        }
        vkDestroySwapchainKHR(device_, swap_chain_, nullptr);
    }


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

//
// Created by 潘鑫 on 2026/1/22.
//

// 下面这个只能在一个 cpp 文件中定义
#ifdef ENGINE_USE_VOLK
#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <volk.h>
#endif
#include "vulkan_backend.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "global_singleton.h"
#include "vulkan_read_attribute.h"
#include "image_and_view_paramter.h"
#include "vulkan_execute_command.h"
#include "vulkan_validation_layer.h"

// 必须使用 atomic 保证多线程可见性与禁止指令重排
static std::atomic<VK_backend *> backend_instance{nullptr};
static std::mutex backend_mutex;

VK_backend *VK_backend::instance_ptr = nullptr;


VK_backend &VK_backend::instance() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        instance_ptr = new VK_backend();
    });
    return *instance_ptr;
}

bool VK_backend::destroy_instance() {
    if (instance_ptr != nullptr) {
        delete instance_ptr;
    }
    instance_ptr = nullptr;
    return true;
}

VkExtent2D VK_backend::get_swap_chain_image_extent() const {
    return swap_image_extent_;
}

void VK_backend::create() {
    // 顺序不能更改
    create_instance();
    create_surface();
    choose_one_physical_device();
    create_device();
    create_VMA();
    update_current_extent();
    create_swap_chain(VK_NULL_HANDLE);
    create_depth_format();
    create_render_image();
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
}

void VK_backend::create_depth_format() {
    std::vector<VkFormat> depthFormatList{VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    for (VkFormat &format: depthFormatList) {
        VkFormatProperties2 formatProperties{.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
        vkGetPhysicalDeviceFormatProperties2(physical_device_, format, &formatProperties);
        if (formatProperties.formatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            depth_format_ = format;
            break;
        }
    }
}

void VK_backend::destroy_swap_chain(VkSwapchainKHR old_swap_chain) {
    if (old_swap_chain == swap_chain_ && old_swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, old_swap_chain, nullptr);
        swap_chain_ = VK_NULL_HANDLE;
    } else if (old_swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, old_swap_chain, nullptr);
    }
}

void VK_backend::copy_swap_chain_image_to_screen() {
    frame_index_ = (frame_index_ + 1) % maxFramesInFlight;
    VkPresentInfoKHR presentInfo{
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &get_can_render_to_image_semaphores()[swap_chain_image_index_], // 不需要++ ？？可以，
        .swapchainCount     = 1,
        .pSwapchains        = &get_swap_chain(),
        .pImageIndices      = &swap_chain_image_index_
    }; {
        const auto result = Command_submit_manager::command_copy_image_to_screen(presentInfo);
        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
            // 成功或者 因为当前屏幕已经改变,但是已经能够渲染完当前画面
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // 目前还没有走到这里
            destroy_and_recreate_swap_chain_and_image();
            destroy_and_recreate_fence_and_semaphore();
        } else {
            LOG_ERROR(g_log(), " Fatal error VkResult is {}  ", VK_error_string(result));
        }
    }
}

void VK_backend::destroy_and_recreate_swap_chain_and_image() {
    set_screen_window_size_changed(false);
    const auto old_swap_chain = get_swap_chain();
    create_swap_chain(old_swap_chain);
    destroy_render_image();
    create_render_image();
    destroy_swap_chain(old_swap_chain);
}


void VK_backend::create_fences() {
    const auto &backend = VK_backend::instance();
    VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateFence(backend.get_device(), &fenceCI, nullptr, &fences_[i]));
    }
}

void VK_backend::destroy_fences() {
    for (auto i = 0; i < maxFramesInFlight; i++) {
        const auto result = vkGetFenceStatus(get_device(), fences_[i]);
        if (result == VK_SUCCESS) {
        } else if (result == VK_NOT_READY) {
            VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(device_));
            // 部分情况下是有用的,但是
        } else if (result == VK_ERROR_DEVICE_LOST) {
        } else {
        }
        vkDestroyFence(get_device(), fences_[i], nullptr); //  这里还需要
        fences_[i] = VK_NULL_HANDLE;
    }
    return;
}


void VK_backend::destroy_and_recreate_fence_and_semaphore() {
    destroy_fences();
    destroy_present_Semaphores();
    destroy_renderSemaphores();
    create_fences();
    create_present_Semaphores();
    create_renderSemaphores();
    swap_chain_image_index_ = 0;
}

void VK_backend::create_present_Semaphores() {
    const auto &backend = VK_backend::instance();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto i = 0; i < maxFramesInFlight; i++) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(backend.get_device(), &semaphoreCI,
                                     nullptr, &present_semaphores_[i]));
    }
}

void VK_backend::destroy_present_Semaphores() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkDestroySemaphore(backend.get_device(), present_semaphores_[i], nullptr); //
        present_semaphores_[i] = VK_NULL_HANDLE;
    }
}


[[nodiscard]] size_t VK_backend::get_swap_chain_images_size() const {
    return swap_chain_images_.size();
}


const VKR_image_ptr &VK_backend::get_blank_swap_chain_image() {
    // forces the CPU to stop and wait until the GPU has finished executing a specific batch of commands
    VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(get_device(),
                                 1,
                                 &get_current_fences(),
                                 true,
                                 UINT64_MAX));
    VK_CHECK_RESULT_NOT_EXIT(vkResetFences(get_device(),
                                 1,
                                 &get_current_fences()));
    auto result = vkAcquireNextImageKHR(get_device(),
                                        get_swap_chain(),
                                        UINT64_MAX,
                                        get_current_presentSemaphores(),
                                        VK_NULL_HANDLE,
                                        &swap_chain_image_index_); // 其实是在这里执行了 ++ 的工作
    if (result == VK_SUCCESS) {
        return swap_chain_images_.at(get_swap_chain_image_index());
    } else if (result == VK_ERROR_OUT_OF_DATE_KHR ||
               result == VK_SUBOPTIMAL_KHR ||
               is_screen_window_size_changed()) {
        // 屏幕的大小发生了变化, 需要等待什么? 之前的提交全部绘制完成
        // 已经绘制完成的,全部显示到屏幕上完成
        VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(device_));
        destroy_and_recreate_swap_chain_and_image();
        destroy_and_recreate_fence_and_semaphore();
        VK_CHECK_RESULT_NOT_EXIT(vkWaitForFences(get_device(),
                                     1,
                                     &get_current_fences(),
                                     true,
                                     UINT64_MAX));
        VK_CHECK_RESULT_NOT_EXIT(vkResetFences(get_device(), 1, &get_current_fences()));
        result = vkAcquireNextImageKHR(get_device(),
                                       get_swap_chain(),
                                       UINT64_MAX,
                                       get_current_presentSemaphores(),
                                       VK_NULL_HANDLE,
                                       &swap_chain_image_index_);
        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
            // 大小还在变动 的情况下 在这里处理掉了
            return swap_chain_images_.at(get_swap_chain_image_index());
        } else {
            // 这里就是再次申请还是不成功, 那么需要做什么呢?
            // 有几种情况,大小还在变动,所以不成功
            //          vulkan 设备错误,但是不能成功
        }
    } else {
        LOG_ERROR(g_log(), " failed to acquire swap chain image! ");
    }
    return nullptr;
}


uint32_t VK_backend::get_common_queue() const {
    return queueFamilyIndices.graphics;
}

uint32_t VK_backend::get_compute_queue() const {
    return queueFamilyIndices.compute;
}

uint32_t VK_backend::get_transfer_queue() const {
    return queueFamilyIndices.transfer;
}

const VkInstance &VK_backend::get_instance() const {
    return instance_;
}

const VkDevice &VK_backend::get_device() const {
    return device_;
}

const VkPhysicalDevice &VK_backend::get_physical_device() const {
    return physical_device_;
}

const VkQueue &VK_backend::get_queue() const {
    return graphics_queue_;
}

const VkSurfaceKHR &VK_backend::get_surface() const {
    return surface_;
}

const VkSwapchainKHR &VK_backend::get_swap_chain() const {
    return swap_chain_;
}

const VmaAllocator &VK_backend::get_allocator() const {
    return allocator_;
}

SDL_Window *VK_backend::get_window() const {
    return window_;
}

VkSurfaceCapabilitiesKHR VK_backend::get_surface_caps() const {
    VkSurfaceCapabilitiesKHR surface_caps_{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps_);
    return surface_caps_;
}

VK_backend::~VK_backend() {
    volkFinalize();
}


void VK_backend::create_instance() {
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

    if (!instanceExtensions.empty()) {
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


void VK_backend::create_surface() {
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    int display_count = 0;
    // 1. 获取当前所有连接的显示器 ID 数组（SDL3 推荐写法）
    SDL_DisplayID *displays = SDL_GetDisplays(&display_count);

    if (displays && display_count > 0) {
        // 2. 获取主显示器（数组第一个元素）的当前显示模式
        const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(displays[0]);
        if (mode) {
            int screen_w = mode->w; // 屏幕逻辑宽度
            int screen_h = mode->h; // 屏幕逻辑高度

            // Create window with Vulkan graphics context
            float main_scale             = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
            SDL_WindowFlags window_flags =
                    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
            window_ = SDL_CreateWindow("Dear ImGui SDL3+Vulkan example", (int) (screen_w * main_scale * 0.667),
                                       (int) (screen_h * main_scale), window_flags);
            if (window_ == nullptr) {
                printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
                return;
            }
            refresh_rate_ = mode->refresh_rate;
            SDL_SetWindowPosition(window_, screen_w * main_scale * 0.333, SDL_WINDOWPOS_CENTERED);
            SDL_ShowWindow(window_);
        }
        // 4. 必须手动释放 SDL3 分配的显示器数组内存
        SDL_free(displays);
    }

    // Create Window Surface
    if (SDL_Vulkan_CreateSurface(window_, instance_, VK_ORIGINAL_Allocator, &surface_) == 0) {
        printf("Failed to create Vulkan surface.\n");
        return;
    }
}

bool VK_backend::choose_one_physical_device() {
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
                VkPhysicalDeviceProperties2 deviceProperties{
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
                };
                vkGetPhysicalDeviceProperties2(physical_device_, &deviceProperties);
                LOG_INFO(g_log(), "Selected device:  {}!", deviceProperties.properties.deviceName);
                return true;
            }
            queueFamilyIndex++;
        }
    }
    return true;
}


bool VK_backend::set_screen_window_size_changed(const bool value) {
    framebufferResized = value;
    return framebufferResized;
}

bool VK_backend::is_screen_window_size_changed() const {
    return framebufferResized;
}

float VK_backend::get_refresh_rate() const {
    return refresh_rate_;
}

void VK_backend::create_device() {
    // Logical device

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    const float defaultQueuePriority(0.0f); // 1.0 表示最高优先级，0.0 表示最低优先级
    VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT;
    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
        queueFamilyIndices.graphics = getQueueFamilyIndex(physical_device_, VK_QUEUE_GRAPHICS_BIT);
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
        queueFamilyIndices.compute = getQueueFamilyIndex(physical_device_, VK_QUEUE_COMPUTE_BIT);
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
        queueFamilyIndices.transfer = getQueueFamilyIndex(physical_device_, VK_QUEUE_TRANSFER_BIT);
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


    VkPhysicalDeviceVulkan11Features enabledVk1Features{
        .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext                = nullptr,
        .storageInputOutput16 = true,
        .shaderDrawParameters = true,
        // gl_DrawID：当前绘制命令在本次批量绘制（Multi-Draw）中的索引（从 0 开始计数）。
        // gl_BaseVertex：当前绘制命令中指定的顶点索引偏移量（对应 C++ 中 VkDrawIndexedIndirectCommand::vertexOffset）。
        // gl_BaseInstance：当前绘制命令中指定的实例偏移量（对应 C++ 中 VkDrawIndexedIndirectCommand::firstInstance）。
    };

    VkPhysicalDeviceVulkan12Features enabledVk12Features{
        .sType                                        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext                                        = &enabledVk1Features,
        .shaderFloat16                                = true,
        .shaderInt8                                   = true,
        .descriptorIndexing                           = true,
        .shaderSampledImageArrayNonUniformIndexing    = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingUpdateUnusedWhilePending    = true,
        .descriptorBindingPartiallyBound              = true,
        .descriptorBindingVariableDescriptorCount     = true,
        .runtimeDescriptorArray                       = true,
        .scalarBlockLayout                            = true,
        .timelineSemaphore                            = true,
        .bufferDeviceAddress                          = true,
        .shaderOutputViewportIndex                    = true,
        .shaderOutputLayer                            = true,
    };
    VkPhysicalDeviceVulkan13Features enabledVk13Features{
        .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext            = &enabledVk12Features,
        .synchronization2 = true,
        .dynamicRendering = true,
    };

    std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME};
    deviceExtensions.push_back("VK_KHR_portability_subset");

    const VkPhysicalDeviceFeatures enabledVk10Features{
        .multiDrawIndirect                      = VK_TRUE,
        .samplerAnisotropy                      = VK_TRUE,
        .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
        .shaderInt64                            = VK_TRUE,
        .shaderInt16                            = VK_TRUE,
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

void VK_backend::create_VMA() {
    // VMA
    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr   = vkGetDeviceProcAddr,
        .vkCreateImage         = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorCI{
        .flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice   = physical_device_,
        .device           = device_,
        .pVulkanFunctions = &vkFunctions,
        .instance         = instance_,
    };
    VK_CHECK_RESULT(vmaCreateAllocator(&allocatorCI, &allocator_));
}


void VK_backend::create_swap_chain(VkSwapchainKHR old_swap_chain) {
    // Swap chain
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    const VkExtent2D extent = get_swap_chain_image_extent();

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
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_STORAGE_BIT |
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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

std::vector<VKR_image_ptr> VK_backend::create_swap_chain_image_and_view() {
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physical_device_, surface_);
    uint32_t imageCount{0};
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, nullptr));

    std::vector<VKR_image_ptr> result;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    images.resize(imageCount);
    image_views.resize(imageCount);
    const VkExtent2D extent = get_swap_chain_image_extent();

    Image_and_view_parameters parameters{};
    parameters.format      = surfaceFormat.format;
    parameters.width       = extent.width;
    parameters.height      = extent.height;
    parameters.depth       = 1;
    parameters.usage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    parameters.aspectMask  = VK_IMAGE_ASPECT_COLOR_BIT;
    parameters.tiling      = VK_IMAGE_TILING_OPTIMAL;
    parameters.mipLevels   = 1;
    parameters.arrayLayers = 1;
    parameters.flags       = 0;

    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device_, swap_chain_, &imageCount, images.data()));
    for (auto i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewCI{
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image    = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = surfaceFormat.format,
            .subresourceRange{
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        };
        VK_CHECK_RESULT(vkCreateImageView(device_, &viewCI, nullptr, &image_views[i]));
    }
    for (auto i = 0; i < imageCount; i++) {
        // 下面的一个可以 将 index 改为零的前提是不能通过 bindless 来采样上一帧 输出的图像
        result.emplace_back(std::make_shared<VKR_image>(images[i],VK_NULL_HANDLE, 0, image_views[i], parameters));
    }
    return result;
}

void VK_backend::check_malloc() const {
    VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
    vmaGetHeapBudgets(allocator_, budgets); // 极快，可每帧调用 //

    // 获取例如 DEVICE_LOCAL 堆的索引数据
    uint32_t heapIndex = 0;
    std::cout << "VMA 账本统计已分配: " << budgets[heapIndex].statistics.allocationBytes / (1024 * 1024) << " MB\n";
    std::cout << "VMA 实际向 Vulkan 申请的 Block 大小: " << budgets[heapIndex].statistics.blockBytes / (1024 * 1024) <<
            " MB\n";
    std::cout << "系统当前实时底层 Usage 估算: " << budgets[heapIndex].usage / (1024 * 1024) << " MB\n";

#ifndef NDEBUG
    VmaTotalStatistics pStats;
    vmaCalculateStatistics(allocator_, &pStats);
#endif
}

void VK_backend::destroy() {
    destroy_render_image();
    constexpr uint64_t finished_timeline = std::numeric_limits<uint64_t>::max();
    discard_image_and_view_map_clean(finished_timeline);
    destroy_renderSemaphores();
    destroy_fences();
    destroy_present_Semaphores();


    if (swap_chain_ != VK_NULL_HANDLE)
        destroy_swap_chain(swap_chain_);

    if (instance_ != VK_NULL_HANDLE && surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
    if (allocator_ != VK_NULL_HANDLE) {
        VmaTotalStatistics stats;
        vmaCalculateStatistics(allocator_, &stats);
        // 获取全局未销毁的分配总数
        uint32_t activeAllocCount = stats.total.statistics.allocationCount;


        vmaDestroyAllocator(allocator_);
        allocator_ = VK_NULL_HANDLE;
    }
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
    if (window_ != VK_NULL_HANDLE) {
        SDL_DestroyWindow(window_);
        SDL_Quit();
    }
}

void VK_backend::update_current_extent() {
    swap_image_extent_ = get_swap_image_rational_extent(physical_device_, surface_, window_);
}

VkExtent2D VK_backend::get_current_extent() const {
    return swap_image_extent_;
}

void VK_backend::create_renderSemaphores() {
    const auto &backend = VK_backend::instance();
    VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    render_to_image_semaphores_.resize(get_swap_chain_images_size());
    LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
    for (auto &semaphore: render_to_image_semaphores_) {
        VK_CHECK_RESULT_NOT_EXIT(vkCreateSemaphore(backend.get_device(), &semaphoreCI, nullptr, &semaphore));
    }
}


void VK_backend::destroy_renderSemaphores() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
        vkDestroySemaphore(backend.get_device(), render_to_image_semaphores_[i], nullptr);
        render_to_image_semaphores_[i] = VK_NULL_HANDLE;
    }
}

void VK_backend::destroy_render_image() {
    for (const auto &image: swap_chain_images_) {
        image->destroy_image();
    }
    swap_chain_images_.clear();
}

void VK_backend::create_render_image() {
    swap_chain_images_ = create_swap_chain_image_and_view();
}


const VkFormat &VK_backend::get_image_format() const {
    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physical_device_, surface_);
    return surfaceFormat.format;
}

const VkFormat &VK_backend::get_depth_format() const {
    return depth_format_;
}

/**
 * 苹果很特殊。给了 4096 个字节，amd 最小给 128 个字节，nvidia 最小给 256 个字节
 * @return
 */
uint32_t get_maxPushConstantsSize() {
    const auto &backend = VK_backend::instance();
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(backend.get_physical_device(), &properties);

    // 获取最大字节限制
    uint32_t maxSize = properties.limits.maxPushConstantsSize;
    return maxSize;
}

/**
 * 苹果电脑关于采样器是有限制的，最大是1024个，也就是1024，只是对 after 有限制
 * @return
 */
uint32_t get_max_descriptor_update_after_bind_samplers() {
    // 准备结构体链
    const auto &backend = VK_backend::instance();

    VkPhysicalDeviceDescriptorIndexingProperties indexingProps{};
    indexingProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;

    VkPhysicalDeviceProperties2 deviceProps{};
    deviceProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProps.pNext = &indexingProps;

    // 查询属性
    vkGetPhysicalDeviceProperties2(backend.get_physical_device(), &deviceProps);

    // 现在你可以获取最大值，例如支持 100,000+ 的纹理绑定
    uint32_t maxBindlessTextures = indexingProps.maxPerStageDescriptorUpdateAfterBindSampledImages;
    return maxBindlessTextures;
}


void get_support_texture_formats() {
    const auto &backend = VK_backend::instance();


    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(backend.get_physical_device(), &supportedFeatures);
    bool supportsBC   = supportedFeatures.textureCompressionBC;   // 支持 BC7, BC3 等 (PC 常用)
    bool supportsETC2 = supportedFeatures.textureCompressionETC2; // 支持 ETC2 (移动端常用)
    bool supportsASTC = supportedFeatures.textureCompressionASTC_LDR;

    // ASTC      >     BC7      >        ETC2
    // 全能王者      桌面端画质巅峰     移动端基石/低保底
}

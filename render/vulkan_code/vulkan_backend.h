//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_GLFW_VULKAN_H
#define HELLO_MAC_GLFW_VULKAN_H

#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "global_singleton.h"
#include "GPU_backend.h"


#include "vulkan_image.h"


class VK_backend : public GPU_backend {
private:
    // ApplicationInfo 的参数
    std::string application_name_ = "Vulkan Example";
    std::string engine_name_      = "no engine";
    uint32_t application_version_ = 102;
    uint32_t engine_version_      = 0;
    uint32_t api_version_         = VK_API_VERSION_1_3;


    std::vector<const char *> instanceExtensions;

    // 需要给外部看到的变量，添加函数给出
    GLFWwindow *window_               = nullptr;
    VkInstance instance_              = VK_NULL_HANDLE;
    VkSurfaceKHR surface_             = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_                  = VK_NULL_HANDLE;
    VkQueue graphics_queue_           = VK_NULL_HANDLE;
    VkQueue present_queue_            = VK_NULL_HANDLE;
    VkQueue transfer_queue_           = VK_NULL_HANDLE;
    VkQueue compute_queue_            = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_        = VK_NULL_HANDLE;
    VmaAllocator allocator_           = VK_NULL_HANDLE; // 之后需要添加的另一个项目中


    /**
     * frameIndex 正在渲染的一帧图像
     * imageIndex swap chain 创建的 image 的索引
     * 可能会有三个 image 交替显示到屏幕
     * 但是永远只有 一个 image 用于渲染
     * 屏幕绘制比较快的话，三缓冲没有太大的作用
    * 屏幕绘制比较慢的话，丢弃过时帧，选择最新帧绘制，开始渲染到开始显示的延迟的延迟不一致的问题
     */


public:
    // 为什么会多一个这个？   内存屏障的时候需要用到，清理的时候不用清理，由swap chain 清理

    uint32_t queue_family_{0}; // 不清楚是否能够删除
    VkFormat depth_format_{VK_FORMAT_UNDEFINED};

    struct {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queueFamilyIndices{}; // 不用给到外部，

    bool framebufferResized = false;

public:
    bool set_frame_buffer_resize(const bool value) {
        framebufferResized = value;
        return framebufferResized;
    };

    [[nodiscard]] bool is_frame_buffer_resize() const {
        return framebufferResized;
    };

    uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

    void create_instance();

    void create_surface();

    bool choose_one_physical_device();

    void create_device();

    void create_VMA();

    void create_swap_chain(VkSwapchainKHR old_swap_chain);

    std::vector<VKR_image_ptr> create_swap_chain_image_and_view();

    VKR_image_ptr create_G_buffer_image_and_view(VkFormat g_buffer_format, VkImageUsageFlagBits usage) const;

    VKR_image_ptr create_depth_image_and_view();

    static VK_backend &get();

    void init_device_handle() {
        // 顺序不能更改
        create_instance();
        create_surface();
        choose_one_physical_device();
        create_device();
        create_VMA();
        create_swap_chain(VK_NULL_HANDLE);
        create_swap_chain_image_and_view();
    }

    void destroy_swap_chain(VkSwapchainKHR old_swap_chain) const {
        vkDestroySwapchainKHR(device_, old_swap_chain, nullptr);
    }

    void destroy();

    [[nodiscard]] VkExtent2D get_current_extent() const {
        const VkExtent2D extent = get_swap_image_rational_extent(physical_device_, surface_, window_);
        return extent;
    }

    [[nodiscard]] VkViewport get_viewport() const {
        auto temp_extent = get_current_extent();

        VkViewport viewport{
            .width    = static_cast<float>(temp_extent.width),
            .height   = static_cast<float>(temp_extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        return viewport;
    }

    [[nodiscard]] VkRect2D get_scissor() const {
        const auto temp_extent = get_current_extent();
        VkRect2D scissor{
            .extent = temp_extent,
        };
        return scissor;
    }

    [[nodiscard]] const VkFormat &get_image_format() const {
        VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physical_device_, surface_);
        return surfaceFormat.format;
    }

    [[nodiscard]] const VkFormat &get_depth_format() const {
        return depth_format_;
    }


    [[nodiscard]] uint32_t get_queue_Family() const {
        return queue_family_;
    }

    [[nodiscard]] const VkInstance &get_instance() const {
        return instance_;
    }

    [[nodiscard]] const VkDevice &get_device() const {
        return device_;
    }

    [[nodiscard]] const VkPhysicalDevice &get_physical_device() const {
        return physical_device_;
    }

    [[nodiscard]] const VkQueue &get_queue() const {
        return graphics_queue_;
    }

    [[nodiscard]] const VkSurfaceKHR &get_surface() const {
        return surface_;
    }

    [[nodiscard]] const VkSwapchainKHR &get_swap_chain() const {
        return swap_chain_;
    }


    [[nodiscard]] const VmaAllocator &get_allocator() const {
        return allocator_;
    }

    [[nodiscard]] GLFWwindow *get_window() const {
        return window_;
    }


    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_caps() const {
        VkSurfaceCapabilitiesKHR surface_caps_{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps_);
        return surface_caps_;
    }

private:
    VK_backend() = default;

    ~VK_backend();

public:
    VK_backend(const VK_backend &) = delete;

    VK_backend &operator=(const VK_backend &) = delete;

    VK_backend(VK_backend &&) = delete;

    VK_backend &operator=(VK_backend &&) = delete;
};

uint32_t get_maxPushConstantsSize();

uint32_t get_max_descriptor_update_after_bind_samplers();

void get_support_texture_formats();
#endif //HELLO_MAC_GLFW_VULKAN_H

//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_GLFW_VULKAN_H
#define HELLO_MAC_GLFW_VULKAN_H

#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include "global_singleton.h"
#include "GPU_backend.h"
#include "vulkan_image.h"
#include "vulkan_read_attribute.h"
#include <SDL3/SDL.h>


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
    float refresh_rate_               = 30.0f;
    SDL_Window *window_               = nullptr;
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
    VkExtent2D extent_;

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
    bool set_frame_buffer_resize(const bool value);;

    [[nodiscard]] bool is_frame_buffer_resize() const;;

    float get_refresh_rate() const;

    uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

    void create_instance();

    void create_surface();

    bool choose_one_physical_device();

    void create_device();

    void create_VMA();

    void create_swap_chain(VkSwapchainKHR old_swap_chain);

    std::vector<VKR_image_ptr> create_swap_chain_image_and_view();

    static VK_backend &instance();

    VkExtent2D get_swap_rational_extent() const;

    void create();

    void create_depth_format();

    void destroy_swap_chain(VkSwapchainKHR old_swap_chain);

    void destroy();

    void update_current_extent();

    [[nodiscard]] VkExtent2D get_current_extent() const;

    [[nodiscard]] VkViewport get_viewport(bool flip_y_axis = false) const;

    [[nodiscard]] VkRect2D get_scissor() const;

    [[nodiscard]] const VkFormat &get_image_format() const;

    [[nodiscard]] const VkFormat &get_depth_format() const;

    [[nodiscard]] uint32_t get_queue_Family() const;

    [[nodiscard]] const VkInstance &get_instance() const;

    [[nodiscard]] const VkDevice &get_device() const;

    [[nodiscard]] const VkPhysicalDevice &get_physical_device() const;

    [[nodiscard]] const VkQueue &get_queue() const;

    [[nodiscard]] const VkSurfaceKHR &get_surface() const;

    [[nodiscard]] const VkSwapchainKHR &get_swap_chain() const;

    [[nodiscard]] const VmaAllocator &get_allocator() const;

    [[nodiscard]] SDL_Window *get_window() const;

    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_caps() const;

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

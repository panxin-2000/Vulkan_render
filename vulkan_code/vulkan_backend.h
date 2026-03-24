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

#include "engine.h"
#include "global_singleton.h"



#include "vulkan_image.h"



class VK_backend {
private:
    // ApplicationInfo 的参数
    std::string application_name_ = "Vulkan Example";
    std::string engine_name_      = "no engine";
    uint32_t application_version_ = 102;
    uint32_t engine_version_      = 0;
    uint32_t api_version_         = VK_API_VERSION_1_3;


    std::vector<const char *> instanceExtensions;

    // 需要给外部看到的变量，添加函数给出
    GLFWwindow *window_                = nullptr;
    VkInstance instance_               = VK_NULL_HANDLE;
    VkSurfaceKHR surface_              = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_  = VK_NULL_HANDLE;
    VkDevice device_                   = VK_NULL_HANDLE;
    VkQueue graphics_queue_            = VK_NULL_HANDLE;
    VkQueue present_queue_             = VK_NULL_HANDLE;
    VkQueue transfer_queue_            = VK_NULL_HANDLE;
    VkQueue compute_queue_             = VK_NULL_HANDLE;
    VkSemaphore vk_timeline_semaphore_ = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_         = VK_NULL_HANDLE;
    VmaAllocator allocator_            = VK_NULL_HANDLE; // 之后需要添加的另一个项目中


    /**
     * frameIndex 正在渲染的一帧图像
     * imageIndex swap chain 创建的 image 的索引
     * 可能会有三个 image 交替显示到屏幕
     * 但是永远只有 一个 image 用于渲染
     * 屏幕绘制比较快的话，三缓冲没有太大的作用
    * 屏幕绘制比较慢的话，丢弃过时帧，选择最新帧绘制，开始渲染到开始显示的延迟的延迟不一致的问题
     */


public:
    Engine engine_;

    void engine_init() {
        engine_.engine_init();
        create_timeline_Semaphores();
    }

    void engine_destroy() {
        engine_.engine_destroy();
    }


    [[nodiscard]] uint64_t get_finished_timeline() const {
        uint64_t current_timeline;
        VkResult result = vkGetSemaphoreCounterValue(get_device(), vk_timeline_semaphore_, &current_timeline);
        assert(result == VK_SUCCESS && "vulkan get timeline semaphore value error");
        return current_timeline;
    }

    [[nodiscard]] const VkImage &get_current_swap_chain_image() const;

    [[nodiscard]] const VkImageView &get_current_swap_image_view() const;

    [[nodiscard]] const VkImage &get_current_depth_image() const;

    [[nodiscard]] const VkImageView &get_current_depth_view() const;

    [[nodiscard]] const VkImage &get_current_position_image() const;

    [[nodiscard]] const VkImageView &get_current_position_view() const;

    [[nodiscard]] const VkImage &get_current_normal_image() const;

    [[nodiscard]] const VkImageView &get_current_normal_view() const;

    [[nodiscard]] const VkImage &get_current_baseColor_image() const;

    [[nodiscard]] const VkImageView &get_current_baseColor_view() const;


    void create_timeline_Semaphores();

    void submit_render_queue(uint64_t time_line);

    static uint64_t get_current_submit_timeline() {
        static std::atomic<uint64_t> time_line = 1;
        ++time_line;
        return time_line - 1; // 第一次拿到的时候就是 1
    }


    void get_image_to_render();


    void copy_image_to_screen();


    // 为什么会多一个这个？   内存屏障的时候需要用到，清理的时候不用清理，由swap chain 清理
    std::vector<VKR_image_ptr> swap_chain_images_;
    std::vector<VKR_image_ptr> G_buffer_Position_images_;
    std::vector<VKR_image_ptr> g_buffer_Normal_images_;
    std::vector<VKR_image_ptr> G_buffer_BaseColor_images_;
    std::vector<VKR_image_ptr> depth_images_;

    uint32_t queue_family_{0}; // 不清楚是否能够删除
    VkFormat depth_format_{VK_FORMAT_UNDEFINED};

    struct {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queueFamilyIndices{}; // 不用给到外部，

    bool framebufferResized = false;

private:
    uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

    void create_instance();

    void create_surface();

    bool choose_one_physical_device();

    void create_device();

    void create_VMA();

    void create_swap_chain(VkSwapchainKHR old_swap_chain);

    void create_swap_chain_image_and_view();

    VKR_image_ptr create_G_buffer_image_and_view(VkFormat g_buffer_format, VkImageUsageFlagBits usage) const;

    VKR_image_ptr create_depth_image_and_view();

public:
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
        depth_images_.push_back(create_depth_image_and_view());
        depth_images_.push_back(create_depth_image_and_view());

        G_buffer_Position_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_Position_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        g_buffer_Normal_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        g_buffer_Normal_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_BaseColor_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R8G8B8A8_UNORM,
                                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_BaseColor_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R8G8B8A8_UNORM,
                                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    }

    void recreate_swap_chain() {
        // std::cout << "recreate_swap_chain" << std::endl;
        framebufferResized = false;
        vkDeviceWaitIdle(device_);
        const auto old_swap_chain = swap_chain_;
        create_swap_chain(old_swap_chain);
        for (const auto &image: depth_images_) {
            image->destroy_image();
        }
        for (const auto &image: swap_chain_images_) {
            image->destroy_image();
        }
        for (const auto &image: G_buffer_Position_images_) {
            image->destroy_image();
        }
        for (const auto &image: g_buffer_Normal_images_) {
            image->destroy_image();
        }
        for (const auto &image: G_buffer_BaseColor_images_) {
            image->destroy_image();
        }
        vkDestroySwapchainKHR(device_, old_swap_chain, nullptr);

        create_swap_chain_image_and_view();
        depth_images_.push_back(create_depth_image_and_view());
        depth_images_.push_back(create_depth_image_and_view());
        G_buffer_Position_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_Position_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        g_buffer_Normal_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        g_buffer_Normal_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_BaseColor_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R8G8B8A8_UNORM,
                                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_BaseColor_images_.push_back(create_G_buffer_image_and_view(VK_FORMAT_R8G8B8A8_UNORM,
                                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
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

    [[nodiscard]] const std::vector<VKR_image_ptr> &get_swap_chain_images() const {
        return swap_chain_images_;
    }

    [[nodiscard]] const std::vector<VKR_image_ptr> &get_depth_images() const {
        return depth_images_;
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

#endif //HELLO_MAC_GLFW_VULKAN_H

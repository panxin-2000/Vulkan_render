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
    static VK_backend *instance_ptr;

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
    VkQueue transfer_queue_           = VK_NULL_HANDLE;
    VkQueue compute_queue_            = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_        = VK_NULL_HANDLE;
    VmaAllocator allocator_           = VK_NULL_HANDLE; // 之后需要添加的另一个项目中
    VkExtent2D swap_image_extent_;

    std::vector<VKR_image_ptr> swap_chain_images_;
    std::vector<VkSemaphore> render_to_image_semaphores_           = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_ = {};
    std::array<VkFence, maxFramesInFlight> fences_                 = {};

    uint32_t swap_chain_image_index_ = 0;
    uint32_t frame_index_            = 0;

    VkFormat depth_format_{VK_FORMAT_UNDEFINED};

    struct {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queueFamilyIndices{}; // 不用给到外部，

    bool framebufferResized = false;


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


    /*********************** fence_and_semaphore ****************************/


    std::vector<VkSemaphore> &get_can_render_to_image_semaphores() {
        return render_to_image_semaphores_;
    }

    VkSemaphore &get_current_renderSemaphores() {
        return get_can_render_to_image_semaphores()[frame_index_];
    }

    std::array<VkSemaphore, maxFramesInFlight> &get_presentSemaphores() {
        return present_semaphores_;
    }

    VkSemaphore &get_current_presentSemaphores() {
        return get_presentSemaphores()[frame_index_];
    }

    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences_;
    }

    VkFence &get_current_fences() {
        return get_fences()[frame_index_];
    }


    /*********************** swap_chain_images ****************************/

    uint32_t get_swap_chain_image_index() const {
        return swap_chain_image_index_;
    }

    std::vector<VKR_image_ptr> create_swap_chain_image_and_view();

    const VKR_image_ptr &get_blank_swap_chain_image();

    [[nodiscard]] size_t get_swap_chain_images_size() const;

    void copy_swap_chain_image_to_screen();

    VkExtent2D get_swap_chain_image_extent() const;


    /*********************** screen_window ****************************/

    bool set_screen_window_size_changed(const bool value);

    [[nodiscard]] bool is_screen_window_size_changed() const;

    /*********************** other ****************************/

    float get_refresh_rate() const;

    static VK_backend &instance();

    static bool destroy_instance();

    void create();

    void destroy();

    void check_malloc() const;

    /**
     *  有两次调用
     *      渲染循环每次循环的开始需要执行一次
     *      backend 初始化的时候需要执行一次
     */
    void update_current_extent();


    /*********************** 一系列的 get 函数 ****************************/


    [[nodiscard]] VkExtent2D get_current_extent() const;

    [[nodiscard]] const VkFormat &get_image_format() const;

    [[nodiscard]] const VkFormat &get_depth_format() const;

    [[nodiscard]] uint32_t get_common_queue() const;

    [[nodiscard]] uint32_t get_compute_queue() const;

    [[nodiscard]] uint32_t get_transfer_queue() const;

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

    /**************** create ******************/


    void create_instance();

    void create_surface();

    bool choose_one_physical_device();

    void create_device();

    void create_VMA();

    // update_current_extent
    void create_swap_chain(VkSwapchainKHR old_swap_chain);

    void create_depth_format();

    void create_render_image();

    void create_fences();

    void create_present_Semaphores();

    void create_renderSemaphores();


    /**************** destroy ******************/

    void destroy_render_image();

    void destroy_renderSemaphores();

    void destroy_fences();

    void destroy_present_Semaphores();

    void destroy_swap_chain(VkSwapchainKHR old_swap_chain);

    /**************** destroy_and_recreate ******************/

    void destroy_and_recreate_swap_chain_and_image();

    void destroy_and_recreate_fence_and_semaphore();

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

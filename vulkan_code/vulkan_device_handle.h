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


struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

#include "vulkan_image.h"

struct Texture_parameter {
    VKR_image_ptr image;
    VkSampler sampler         = VK_NULL_HANDLE;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class VK_handle {
public:
    // ApplicationInfo 的参数
    std::string application_name_ = "Vulkan Example";
    std::string engine_name_      = "no engine";
    uint32_t application_version_ = 102;
    uint32_t engine_version_      = 0;
    uint32_t api_version_         = VK_API_VERSION_1_3;


    std::vector<const char *> instanceExtensions;


    // 需要给外部看到的变量
    GLFWwindow *window_               = nullptr;
    VkInstance instance_              = VK_NULL_HANDLE;
    VkSurfaceKHR surface_             = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_                  = VK_NULL_HANDLE;
    VkQueue graphics_queue_           = VK_NULL_HANDLE;
    VkQueue present_queue_            = VK_NULL_HANDLE;
    VkQueue transfer_queue_           = VK_NULL_HANDLE;
    VkQueue compute_queue_            = VK_NULL_HANDLE;

    // Engine engine_;

    VkSemaphore vk_timeline_semaphore_ = VK_NULL_HANDLE;

    /**
     * frameIndex 正在渲染的一帧图像
     * imageIndex swap chain 创建的 image 的索引
     * 可能会有三个 image 交替显示到屏幕
     * 但是永远只有 一个 image 用于渲染
     * 屏幕绘制比较快的话，三缓冲没有太大的作用
    * 屏幕绘制比较慢的话，丢弃过时帧，选择最新帧绘制，开始渲染到开始显示的延迟的延迟不一致的问题
     */

    Engine engine_;

public:
    void engine_init() {
        engine_.engine_init();
        create_timeline_Semaphores();
    }

    void engine_destroy() {
        engine_.engine_destroy();
        vkDestroySemaphore(get_device(), vk_timeline_semaphore_, nullptr);
        vk_timeline_semaphore_ = VK_NULL_HANDLE; // 这里设置为 VK_NULL_HANDLE 了，但是上面几个并没有
    }


    [[nodiscard]] uint64_t get_finished_timeline() const {
        uint64_t current_timeline;
        VkResult result = vkGetSemaphoreCounterValue(get_device(), vk_timeline_semaphore_, &current_timeline);
        assert(result == VK_SUCCESS && "vulkan get timeline semaphore value error");
        return current_timeline;
    }


    // std::array<uniform_buffer, maxFramesInFlight> &get_shader_data_buffer() {
    //     return uniform_buffers_;
    // }

    // uniform_buffer &get_current_shader_data_buffer() {
    //     return get_shader_data_buffer()[frameIndex];
    // }


    const VkImage &get_current_swap_chain_image() const;

    const VkImageView &get_current_swap_image_view() const;


    // void create_shader_data_buffer();


    void create_timeline_Semaphores();


    void submit_render_queue(uint64_t time_line);

    static uint64_t get_current_submit_timeline() {
        static std::atomic<uint64_t> time_line = 1;
        ++time_line;
        return time_line - 1; // 第一次拿到的时候就是 1
    }


    /**
     *
     * @param imageIndex 必须用 imageIndex 去找图像资源
     */
    void get_image_to_render();


    void copy_image_to_screen();

    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;

    // 为什么会多一个这个？   内存屏障的时候需要用到，清理的时候不用清理，由swap chain 清理
    std::vector<VKR_image_ptr> swap_chain_images_;

    VKR_image_ptr depth_image_;

    VmaAllocator allocator_ = VK_NULL_HANDLE; // 之后需要添加的另一个项目中
    uint32_t queue_family_{0};                // 不清楚是否能够删除
    VkFormat depth_format_{VK_FORMAT_UNDEFINED};

    struct {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queueFamilyIndices{}; // 不用给到外部，

    bool framebufferResized = false;



    auto &get_bindless_textures() {
        return bindless_textures_;
    }

private:
    std::vector<VkDescriptorImageInfo> bindless_textures_;


    void create_instance();

    // 显示相关
    void create_surface();

    bool choose_one_physical_device();

    uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

    void create_device();

    void create_VMA();

    void create_swap_chain(VkSwapchainKHR old_swap_chain);

    void create_depth_resources();

    void create_swap_chain_image_and_view();

    VKR_image_ptr create_depth_image_and_view();



public:
    static VK_handle &get();



    /**
     * 顺序不能更改
     */
    void init_device_handle() {
        create_instance();
        create_surface();
        choose_one_physical_device();
        create_device();
        create_VMA();
        create_swap_chain(VK_NULL_HANDLE);
        create_swap_chain_image_and_view();
        depth_image_ = create_depth_image_and_view();
    }

    void recreate_swap_chain() {
        std::cout << "recreate_swap_chain" << std::endl;
        framebufferResized = false;
        vkDeviceWaitIdle(device_);
        const auto old_swap_chain = swap_chain_;
        create_swap_chain(old_swap_chain);
        depth_image_->destroy_image();
        for (const auto &image: swap_chain_images_) {
            image->destroy_image();
        }
        vkDestroySwapchainKHR(device_, old_swap_chain, nullptr);

        create_swap_chain_image_and_view();
        depth_image_ = create_depth_image_and_view();
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

    [[nodiscard]] const VkQueue &get_queue() const {
        return graphics_queue_;
    }

    [[nodiscard]] const VkSurfaceKHR &get_surface() const {
        return surface_;
    }

    [[nodiscard]] const VkSwapchainKHR &get_swap_chain() const {
        return swap_chain_;
    }


    [[nodiscard]] VkImage get_depth_image() const {
        return depth_image_->get_image_handle();
    }

    [[nodiscard]] VkImageView get_depth_image_view() const {
        return depth_image_->get_image_view();
    }

    [[nodiscard]] const VmaAllocator &get_allocator() const {
        return allocator_;
    }

    [[nodiscard]] const GLFWwindow *get_window() const {
        return window_;
    }

    [[nodiscard]] const std::vector<VKR_image_ptr> &get_swap_chain_images() const {
        return swap_chain_images_;
    }

    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_caps() const {
        VkSurfaceCapabilitiesKHR surface_caps_{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps_);
        return surface_caps_;
    }

private:
    VK_handle() = default;

    ~VK_handle();

public:
    VK_handle(const VK_handle &) = delete;

    VK_handle &operator=(const VK_handle &) = delete;

    VK_handle(VK_handle &&) = delete;

    VK_handle &operator=(VK_handle &&) = delete;
};


#endif //HELLO_MAC_GLFW_VULKAN_H

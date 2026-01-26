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

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

class VKDevice {
public:
    // ApplicationInfo 的参数
    std::string application_name_ = "Vulkan Example";
    std::string engine_name_ = "no engine";
    uint32_t application_version_ = 102;
    uint32_t engine_version_ = 0;
    uint32_t api_version_ = VK_API_VERSION_1_3;


    std::vector<const char *> instanceExtensions;

    VKDevice();

    ~VKDevice();

    // 需要给外部看到的变量
    GLFWwindow *window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;
    VkQueue transfer_queue_ = VK_NULL_HANDLE;


    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    std::vector<VkImageView> swap_chain_image_views_;

    // 为什么会多一个这个？   内存屏障的时候需要用到，清理的时候不用清理，由swap chain 清理
    std::vector<VkImage> swap_chain_images_;

    VkImage depth_image_;
    VkImageView depth_image_view_;

    VmaAllocator allocator_ = VK_NULL_HANDLE; // 之后需要添加的另一个项目中
    uint32_t queue_family_{0};
    VkFormat depth_format_{VK_FORMAT_UNDEFINED};

public:
    void create_instance();

    void create_surface();

    bool choose_one_physical_device();

    void create_device();

    void create_VMA();

    void create_swap_chain();

    void create_depth_resources();

    void create_swap_chain_image_view();

    void create_depth_image_view();

    void destroy();

    [[nodiscard]] VkExtent2D get_current_extent() const {
        const VkExtent2D extent = get_swap_rational_extent(physical_device_, surface_, window_);
        return extent;
    }

    [[nodiscard]] const VkFormat &get_image_format() const {
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physical_device_, surface_);
        return surfaceFormat.format;
    }

    [[nodiscard]] const VkFormat &get_depth_format() const {
        return depth_format_;
    }


    VmaAllocation depthImageAllocation; // ????? 这是一个什么东西？

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

    [[nodiscard]] const std::vector<VkImageView> &get_swap_image_view() const {
        return swap_chain_image_views_;
    }

    VkImage &get_depth_image() {
        return depth_image_;
    }

    [[nodiscard]] const VkImageView &get_depth_image_view() const {
        return depth_image_view_;
    }

    [[nodiscard]] const VmaAllocator &get_allocator() const {
        return allocator_;
    }

    [[nodiscard]] const GLFWwindow *get_window() const {
        return window_;
    }

    [[nodiscard]] std::vector<VkImage> get_swap_chain_images() const {
        return swap_chain_images_;
    }

    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_caps() const {
        VkSurfaceCapabilitiesKHR surface_caps_{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps_);
        return surface_caps_;
    }
};


#endif //HELLO_MAC_GLFW_VULKAN_H

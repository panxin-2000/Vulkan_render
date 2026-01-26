//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_GLFW_VULKAN_H
#define HELLO_MAC_GLFW_VULKAN_H

#include "vulkan_utility.h"
#include <cassert>
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
    std::string ApplicationName = "Vulkan Example";
    std::string EngineName = "no engine";
    uint32_t applicationVersion = 102;
    uint32_t engineVersion = 0;
    uint32_t apiVersion = VK_API_VERSION_1_3;


    std::vector<const char *> instanceExtensions;

    VKDevice();

    ~VKDevice();

    // 需要给外部看到的变量
    GLFWwindow *window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;


    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<VkImage> swapchainImages; // 为什么会多一个这个？


    VkImage depthImage;
    VkImageView depthImageView;

    VmaAllocator allocator = VK_NULL_HANDLE; // 之后需要添加的另一个项目中
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    const VkFormat imageFormat{VK_FORMAT_B8G8R8A8_SRGB};
    uint32_t queueFamily{0};
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};

public:
    void createInstance();

    void createSurface();

    bool choose_one_physical_device();

    void createDevice();

    void createVMA();

    void create_swapchain();

    void createDepthResources();

    void creare_swapchain_image_view();

    void creare_depth_image_view();

    void destroy();


    // 临时，之后需修改
    VkExtent2D get_current_extent();


    const VkFormat &get_imageFormat() {
        return imageFormat;
    }

    const VkFormat get_depthFormat() {
        return depthFormat;
    }


    VmaAllocation depthImageAllocation; // ????? 这是一个什么东西？

    uint32_t get_queue_Family() {
        return queueFamily;
    }

    const VkInstance &get_instance() {
        return instance_;
    }

    const VkDevice &get_device() {
        return device_;
    }

    const VkQueue &get_queue() {
        return graphicsQueue;
    }

    const VkSurfaceKHR &get_surface() {
        return surface_;
    }

    const VkSwapchainKHR &get_swapchain() const {
        return swapchain;
    }

    const std::vector<VkImageView> &get_swap_image_view() {
        return swapchainImageViews;
    }

    VkImage &get_depth_image() {
        return depthImage;
    }

    const VkImageView &get_depth_image_view() {
        return depthImageView;
    }

    const VmaAllocator &get_allocator() {
        return allocator;
    }

    const GLFWwindow *get_window() {
        return window_;
    }

    std::vector<VkImage> get_swapchain_images() {
        return swapchainImages;
    }

    VkSurfaceCapabilitiesKHR get_surface_caps() {
        return surfaceCaps;
    }
};


#endif //HELLO_MAC_GLFW_VULKAN_H

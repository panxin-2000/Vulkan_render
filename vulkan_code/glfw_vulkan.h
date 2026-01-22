//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_GLFW_VULKAN_H
#define HELLO_MAC_GLFW_VULKAN_H

#include "vulkan_utility.h"
#include <cassert>
#include <string>
#include <vector>

class vulkan_create_screen {
public:
    // ApplicationInfo 的参数
    std::string ApplicationName = "Vulkan Example";
    std::string EngineName = "no engine";
    uint32_t applicationVersion = 102;
    uint32_t engineVersion = 0;
    uint32_t apiVersion = VK_API_VERSION_1_0;


    std::vector<const char *> instanceExtensions;

    vulkan_create_screen();

    ~vulkan_create_screen();

    // 需要给外部看到的变量
    GLFWwindow *window_ = nullptr;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;

private:
    void createInstance();

    void createSurface();

    bool choose_one_physical_device(VkPhysicalDevice &PhysicalDevice);

    void createDevice();
};


#endif //HELLO_MAC_GLFW_VULKAN_H

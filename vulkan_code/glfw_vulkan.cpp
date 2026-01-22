//
// Created by 潘鑫 on 2026/1/22.
//

// 下面这个只能在一个 cpp 文件中定义
#define VOLK_IMPLEMENTATION

#include "glfw_vulkan.h"


vulkan_create_screen::vulkan_create_screen() {
    if (volkInitialize() != VK_SUCCESS) {
        return;
    }
    createInstance();
}

vulkan_create_screen::~vulkan_create_screen() {
}


void vulkan_create_screen::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.pApplicationName = ApplicationName.c_str();
    appInfo.applicationVersion = applicationVersion;
    appInfo.pEngineName = EngineName.c_str();
    appInfo.engineVersion = engineVersion;
    appInfo.apiVersion = apiVersion;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.pApplicationInfo = &appInfo;
    add_platform_need_instance_extensions(instanceCreateInfo, instanceExtensions);
#ifndef NDEBUG //  cmake_build_type 在build 模式下不产生 NDEBUG 宏
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    add_validationLayers(instanceCreateInfo, debugCreateInfo, instanceExtensions);
#else

#endif
    if (instanceExtensions.size() > 0) {
        instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    }
    VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, Allocator,&instance));
}

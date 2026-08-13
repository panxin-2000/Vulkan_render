//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_VULKAN_DEBUG_STRING_H
#define HELLO_MAC_VULKAN_DEBUG_STRING_H
#include "vulkan_backend.h"




struct scoped_debug_label {
    VkCommandBuffer cmd;

    scoped_debug_label(const VkCommandBuffer &cb, const std::string &label) : cmd(cb) {
        VkDebugUtilsLabelEXT labelInfo{};
        labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        labelInfo.pLabelName = label.c_str();
        labelInfo.color[0]   = 1.0f; // R (0.0~1.0)
        labelInfo.color[1]   = 1.0f; // G
        labelInfo.color[2]   = 0.0f; // B (黄色)
        labelInfo.color[3]   = 1.0f; // A
        vkCmdBeginDebugUtilsLabelEXT(cmd, &labelInfo);
    };

    ~scoped_debug_label() {
        vkCmdEndDebugUtilsLabelEXT(cmd);
    };
};


/**
 * @brief 为 Vulkan 对象设置调试名称
 * @param backend
 * @param objectType 对象类型 (如 VK_OBJECT_TYPE_BUFFER)
 * @param handle 对象句柄 (强转为 uint64_t)
 * @param name
 */
inline void SetDebugName(const VK_backend &backend,
                         const VkObjectType objectType,
                         const uint64_t handle,
                         const std::string &name) {
    if (vkSetDebugUtilsObjectNameEXT && !name.empty()) {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType                         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType                    = objectType;
        nameInfo.objectHandle                  = handle;
        nameInfo.pObjectName                   = name.c_str();

        vkSetDebugUtilsObjectNameEXT(backend.get_device(), &nameInfo);
    }
}


inline void gpu_log_label_info(const VkCommandBuffer &cb, const std::string &label) {
    // 添加一个函数 ， 颜色根据不同的类型来确定
    VkDebugUtilsLabelEXT markerInfo{};
    markerInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    markerInfo.pLabelName = label.c_str();
    markerInfo.color[0]   = 1.0f; // R (0.0~1.0)
    markerInfo.color[1]   = 1.0f; // G
    markerInfo.color[2]   = 0.0f; // B (黄色)
    markerInfo.color[3]   = 1.0f; // A
    vkCmdInsertDebugUtilsLabelEXT(cb, &markerInfo);
}

#endif //HELLO_MAC_VULKAN_DEBUG_STRING_H

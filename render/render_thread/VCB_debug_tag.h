//
// Created by 潘鑫 on 2026/8/13.
//

#ifndef HELLO_MAC_VULKAN_DEBUG_STRING_H
#define HELLO_MAC_VULKAN_DEBUG_STRING_H
#include "VCB_vulkan_command_buffer.h"


/**
 * @brief 为 Vulkan 对象设置调试名称
 * @param backend
 * @param objectType 对象类型 (如 VK_OBJECT_TYPE_BUFFER)
 * @param handle 对象句柄 (强转为 uint64_t)
 * @param name
 */
inline void VCB::SetDebugName(const VK_backend &backend,
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


#endif //HELLO_MAC_VULKAN_DEBUG_STRING_H

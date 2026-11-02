//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_VULKAN_GLOBAL_MACRO_H
#define HELLO_MAC_VULKAN_GLOBAL_MACRO_H

#ifdef ENGINE_USE_VOLK
#include "volk.h"
#else
#include <vulkan/vulkan.h>
#endif

#include <string_view>
#include <cassert>
#define VK_ORIGINAL_Allocator nullptr

constexpr uint32_t maxFramesInFlight{2};

inline uint32_t get_max_frames_in_flight() {
    return maxFramesInFlight;
}

// todo : 将res值变成具体的错误字符串
#define VK_CHECK_RESULT_NOT_EXIT(f)					\
{													\
    VkResult res = (f);								\
    if (res != VK_SUCCESS)							\
    {												\
        std::cout << "Fatal : VkResult is \""       \
        << res << "\" in " << __FILE__              \
        << " at line " << __LINE__ << "\n";         \
    }							            		\
}

// todo : 将res值变成具体的错误字符串
#define VK_CHECK_RESULT(f)				            \
{													\
    VkResult res = (f);								\
    if (res != VK_SUCCESS)							\
    {												\
        std::cout << "Fatal : VkResult is \""       \
        << res << "\" in " << __FILE__              \
        << " at line " << __LINE__ << "\n";         \
        assert(res == VK_SUCCESS);			       	\
    }							            		\
}

#define ALIGN_8(size) (((size) + 7) & ~7)
#define ALIGN_16(size) (((size) + 15) & ~15)
#define ALIGN_32(size) (((size) + 31) & ~31)
#define ALIGN_64(size) (((size) + 64) & ~63)
#define ALIGN_128(size) (((size) + 127) & ~127)
#define ALIGN_256(size) (((size) + 255) & ~255)
#define ALIGN_512(size) (((size) + 511) & ~511)
#define ALIGN_1024(size) (((size) + 1023) & ~1023)


inline std::string_view VK_error_string(VkResult errorCode) {
    switch (errorCode) {
#define STR(r) case VK_ ##r: return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default:
            return "UNKNOWN_ERROR";
    }
}


#endif //HELLO_MAC_VULKAN_GLOBAL_MACRO_H

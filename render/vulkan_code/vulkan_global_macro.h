//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_VULKAN_GLOBAL_MACRO_H
#define HELLO_MAC_VULKAN_GLOBAL_MACRO_H

#ifdef ENGINE_USE_VOLK
#include <volk.h>
#else
#include <vulkan/vulkan.h>
#endif


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


#endif //HELLO_MAC_VULKAN_GLOBAL_MACRO_H

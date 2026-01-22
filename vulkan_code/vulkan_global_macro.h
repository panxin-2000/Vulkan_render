//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_VULKAN_GLOBAL_MACRO_H
#define HELLO_MAC_VULKAN_GLOBAL_MACRO_H

#include <volk.h>

#include <iostream>
#include <assert.h>
#define Allocator nullptr


// todo : 将res值变成具体的错误字符串
#define VK_CHECK_RESULT(f)						    \
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

#endif //HELLO_MAC_VULKAN_GLOBAL_MACRO_H

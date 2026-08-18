//
// Created by 潘鑫 on 2026/8/17.
//

#ifndef HELLO_MAC_IMAGE_AND_VIEW_PARAMTER_H
#define HELLO_MAC_IMAGE_AND_VIEW_PARAMTER_H

#include <algorithm>
#include <volk.h>

struct Image_and_view_parameters {
    VkFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    VkImageUsageFlagBits usage;
    VkImageAspectFlags aspectMask;
    VkImageTiling tiling;
    uint32_t mipLevels;


    template<typename H>
    friend H AbslHashValue(H state, const Image_and_view_parameters &sp) {
        // 直接使用 H::combine 把所有成员丢进去，它支持任意数量、任意类型的参数！
        // 并且完美支持原生枚举（如 VkPrimitiveTopology），不需要进行 static_cast 转换
        return H::combine(std::move(state),
                          sp.format,
                          sp.width,
                          sp.height,
                          sp.depth,
                          sp.usage,
                          sp.aspectMask,
                          sp.tiling,
                          sp.mipLevels);
    }

    bool operator==(const Image_and_view_parameters &other) const = default;
};


#endif //HELLO_MAC_IMAGE_AND_VIEW_PARAMTER_H

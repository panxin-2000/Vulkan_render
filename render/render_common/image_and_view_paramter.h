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
    uint32_t arrayLayers;     // sky_box 会使用
    VkImageCreateFlags flags; // sky_box 会使用


    void set_mip_levels(float min_size = 1.0f) {
        // 1. 如果初始尺寸就有任意一边 <= 32，直接判定为 1 级
        if (width <= 32 || height <= 32) {
            mipLevels = 1;
            return;
        }
        // 2. 找到较短的那条边作为“瓶颈瓶颈”
        uint32_t min_dim = std::min(width, height);

        // 3. 一行代码直接算出允许缩放的层数
        // min_dim / 32.0 算出需要缩小的比例，用 log2 算出需要下采样的步数，再向上取整 (ceil)
        mipLevels = static_cast<uint32_t>(std::ceil(std::log2(static_cast<double>(min_dim) / min_size))) + 1;
    }

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
                          sp.mipLevels,
                          sp.flags,
                          sp.arrayLayers);
    }

    bool operator==(const Image_and_view_parameters &other) const = default;
};


#endif //HELLO_MAC_IMAGE_AND_VIEW_PARAMTER_H

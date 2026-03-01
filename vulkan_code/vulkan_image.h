//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_IMAGE_H
#define HELLO_MAC_VULKAN_IMAGE_H
#include "vulkan_global_macro.h"
#include <vk_mem_alloc.h>


class VKR_image {
    VkImage image_handle_     = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;
    VkImageView image_view_   = VK_NULL_HANDLE;
    uint64_t timeline_        = 0;

public:
    VKR_image(const VkImage image_handle,
              const VmaAllocation allocation,
              const VkImageView image_view) : image_handle_(image_handle),
                                              allocation_(allocation),
                                              image_view_(image_view) {
    }

    [[nodiscard]] VkImage get_image_handle(const uint64_t timeline = 0) {
        if (timeline > timeline_) timeline_ = timeline;
        return image_handle_;
    }

    [[nodiscard]] VmaAllocation get_image_allocation(const uint64_t timeline = 0) {
        if (timeline > timeline_) timeline_ = timeline;
        return allocation_;
    }

    void destroy_image();

    ~VKR_image() {
        destroy_image();
    }

    [[nodiscard]] VkImageView get_image_view(const uint64_t timeline = 0) {
        if (timeline > timeline_) timeline_ = timeline;
        return image_view_;
    }
};


class VKR_image_ptr {
public:
    VKR_image_ptr(const VkImage image_handle, const VmaAllocation allocation,
                  const VkImageView image_view) : ptr(std::make_shared<VKR_image>(image_handle, allocation,
                                                               image_view)) {
    }

    VKR_image_ptr() = default;

    ~VKR_image_ptr() {
        ptr = nullptr;
    }

    long use_count() {
        return ptr.use_count();
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }


    void clear() {
        ptr = nullptr;
    }

    VKR_image *operator->() const { return ptr.get(); }

private:
    std::shared_ptr<VKR_image> ptr = nullptr;
};

VkImageView createImageView(const VkImage image,
                            const VkFormat format,
                            const VkImageAspectFlags aspectFlags, uint32_t mipLevels);


void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t mipLevels);

std::pair<VkImage, VmaAllocation> createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                                              VkFormat format,
                                              VkImageTiling tiling, VkImageUsageFlags usage);


#endif //HELLO_MAC_VULKAN_IMAGE_H

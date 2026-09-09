//
// Created by 潘鑫 on 2026/1/23.
//

#ifndef HELLO_MAC_VULKAN_IMAGE_H
#define HELLO_MAC_VULKAN_IMAGE_H
#include "vulkan_global_macro.h"
#include <vk_mem_alloc.h>

#include "APP_utility_mixins.h"
#include "shader_common.h"

#include <readerwriterqueue.h>

#include "image_and_view_paramter.h"
#include "vulkan_buffer.h"

class VKR_image : public NonCopyable {
    VkImage image_handle_     = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;
    VkImageView image_view_   = VK_NULL_HANDLE;
    uint64_t timeline_        = 0;
    uint32_t index_           = 0; //
    Image_and_view_parameters parameters_;

public:
    VKR_image(const VkImage &image_handle,
              const VmaAllocation &allocation,
              const VkImageView &image_view,
              const Image_and_view_parameters &parameters) : image_handle_(image_handle),
                                                             allocation_(allocation),
                                                             image_view_(image_view),
                                                             parameters_(parameters) {
        // 这里开始构建的 时候就需要 添加 index 了
        index_ = get_one_bindless_index();
    }

    const Image_and_view_parameters &get_parameters() const {
        return parameters_;
    }

    uint32_t get_width() const {
        return parameters_.width;
    }

    uint32_t get_height() const {
        return parameters_.height;
    }

    uint32_t get_arrayLayers() const {
        return parameters_.arrayLayers;
    }

    uint32_t get_mipLevels() const {
        return parameters_.mipLevels;
    }

    [[nodiscard]] uint32_t get_index() const {
        return index_;
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

private:
    static moodycamel::BlockingReaderWriterQueue<uint32_t> free_index;
    static std::atomic<uint32_t> max_index;
    // 单入单出， 逻辑线程 和 渲染线程 同时 只会有一个 线程 写入或者释放
    // 其实应该做到 单入 多出 ，这个 才是比较理想的一个状态

    static uint32_t get_one_bindless_index() {
        uint32_t value;
        if (free_index.try_dequeue(value) == true) {
            return value;
        }
        const auto return_value = max_index.load();
        ++max_index;
        return return_value;
    }

    static bool add_to_free_index(const uint32_t &index) {
        free_index.enqueue(index);
        return true;
    }
};


class VKR_image_ptr {
public:
    VKR_image_ptr(const VkImage &image_handle,
                  const VmaAllocation &allocation,
                  const VkImageView &image_view,
                  const Image_and_view_parameters &parameters) : ptr(std::make_shared<VKR_image>(image_handle,
                                                                              allocation,
                                                                              image_view, parameters)) {
    }

    VKR_image_ptr() = default;

    ~VKR_image_ptr() {
        ptr = nullptr; //
    }

    [[nodiscard]] uint32_t use_count() const {
        return ptr.use_count();
    }

    [[nodiscard]] uint32_t get_index() const {
        return ptr->get_index();
    }

    bool operator==(std::nullptr_t) const noexcept {
        return ptr == nullptr;
    }

    // 允许与 nullptr 进行 != 比较 （完美解决你的报错）
    bool operator!=(std::nullptr_t) const noexcept {
        return ptr != nullptr;
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

VKR_image_ptr create_2d_image_and_view(const Image_and_view_parameters &parameters);


void copyBufferToImage(VKR_buffer_ptr buffer,
                       VKR_image_ptr image_ptr,
                       const Image_and_view_parameters &parameters);

void transitionImageLayout(const VKR_image_ptr image_ptr,
                           const Image_and_view_parameters &parameters,
                           const VkImageLayout oldLayout,
                           const VkImageLayout newLayout);

std::pair<VkImage, VmaAllocation> create_2D_Image(uint32_t width, uint32_t height, uint32_t mipLevels,
                                                  VkFormat format,
                                                  VkImageTiling tiling, VkImageUsageFlags usage);

struct Texture_parameter {
    VKR_image_ptr image       = {};
    VkSampler sampler         = VK_NULL_HANDLE;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    [[nodiscard]] VkDescriptorImageInfo get_descriptor_image_info(const uint64_t timeline = 0) const {
        const VkDescriptorImageInfo temp{
            .sampler     = sampler,
            .imageView   = image->get_image_view(timeline),
            .imageLayout = imageLayout
        };
        return temp;
    }
};

void simple_mipmap(const VkCommandBuffer commandBuffer,
                   const VKR_image_ptr image_ptr,
                   const Image_and_view_parameters &parameters);


void transition_image(const VkCommandBuffer commandBuffer,
                      const VkImage image,
                      const uint32_t baseMipLevel,
                      const VkImageLayout oldLayout,
                      const VkImageLayout newLayout,
                      const VkAccessFlags srcAccessMask,
                      const VkAccessFlags dstAccessMask,
                      const VkPipelineStageFlags srcStageMask,
                      const VkPipelineStageFlags dstStageMask);


VKR_buffer_ptr create_image_stage_buffer(VkDeviceSize size,
                                         std::function<void(void *)> mem_copy_callback);

Texture_parameter create_2d_texture(const Picture_parameters &picture_parameters);

VKR_image_ptr create_skybox_texture(std::vector<Picture_parameters> &picture_parameters);

Texture_parameter create_skybox_texture_all(std::vector<std::string> paths);

Texture_parameter create_2d_texture(const std::string &picture_path);

Texture_parameter create_single_color_texture(const uint8_t &R, const uint8_t &G, const uint8_t &B);

Texture_parameter create_texture_from_image(uint8_t *image,
                                            const int &width,
                                            const int &height,
                                            const int &channels);

void discard_image_and_view_map_clean(uint64_t finished_timeline);

Texture_parameter create_2d_texture(const VKR_image_ptr &image_ptr);

Texture_parameter create_compute_image2D_texture(const VKR_image_ptr &image_ptr);

#endif //HELLO_MAC_VULKAN_IMAGE_H

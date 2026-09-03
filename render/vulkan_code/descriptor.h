//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_H
#define HOWTOVULKAN_DESCRIPTOR_H
#include "APP_utility_mixins.h"
#include <volk.h>
#include <vector>

class DescriptorSet_detail : public NonCopyable {
public:
    VkDescriptorSet descriptor_set_          = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_layout_ = VK_NULL_HANDLE;
    uint64_t timeline_                       = 0;

    DescriptorSet_detail(const VkDescriptorSet &descriptor_set, const VkDescriptorSetLayout &layout) {
        descriptor_set_    = descriptor_set;
        descriptor_layout_ = layout;
    }

    //  根据timeline 选择合适的时间释放
    ~DescriptorSet_detail();

    VkDescriptorSet get_descriptor_set(const uint64_t timeline = 0) {
        if (timeline > timeline_) timeline_ = timeline;
        return descriptor_set_;
    }
};

using DescriptorSet_ptr = std::shared_ptr<DescriptorSet_detail>;

using Proxy_descriptor_sets = std::vector<DescriptorSet_ptr>;


// /**
//  * 更新描述符
//  * @param backend
//  * @param textureDescriptors
//  * @param descriptor_set_texture
//  */
// void update_descriptor_sets(const VK_backend &backend, std::vector<VkDescriptorImageInfo> &textureDescriptors,
//                             const std::vector<DescriptorSet_ptr> &descriptor_set_texture);


// std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
// };

auto variable_descriptor(const uint32_t binding_less_size,
                         const std::vector<VkDescriptorBindingFlags> &binding_flags,
                         std::vector<uint32_t> &variableDescCounts);


/**
 * 申请描述符，原本输入的是 单个 descriptor_bindings
 * 需要变更为双缓冲或者多缓冲的结果
 * @param descriptorPool
 * @param descriptor_set_layouts 由 glsl 文件描述的单个 set = 0
 * @param binding_flags
 * @return
 */
Proxy_descriptor_sets
allocate_descriptor_sets(const VkDescriptorPool &descriptorPool,
                         std::vector<VkDescriptorSetLayout> descriptor_set_layouts,
                         const std::vector<VkDescriptorBindingFlags> &binding_flags = {});

void discard_descriptor_set_map_clean(uint64_t current_timeline);

#endif //HOWTOVULKAN_DESCRIPTOR_H

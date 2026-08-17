//
// Created by 潘鑫 on 2026/3/3.
//
#include "descriptor.h"

#include "create_texture.h"
#include "descriptor_organized_sets_and_bindings.h"
#include "descriptor_pool.h"
#include "shader_resolve.h"

std::map<VkDescriptorSet, uint64_t> discard_descriptor_set_map;
std::mutex discard_descriptor_set_map_mutex;

// void update_descriptor_sets(const VK_backend &backend, std::vector<VkDescriptorImageInfo> &textureDescriptors,
//                             const std::vector<DescriptorSet_ptr> &descriptor_set_texture) {
//     std::vector<VkWriteDescriptorSet> writeDescSet;
//     for (uint32_t i = 0; i < descriptor_set_texture.size(); i++) {
//         VkWriteDescriptorSet temp{
//             .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//             .dstSet          = descriptor_set_texture[i]->get_descriptor_set(),
//             .dstBinding      = 0,
//             .descriptorCount = static_cast<uint32_t>(textureDescriptors.size()),
//             .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//             .pImageInfo      = textureDescriptors.data()
//         };
//         writeDescSet.push_back(temp);
//     }
//     std::lock_guard<std::mutex> lock(discard_descriptor_set_map_mutex);
//     vkUpdateDescriptorSets(backend.get_device(),
//                            writeDescSet.size(),
//                            writeDescSet.data(), 0, nullptr);
// }
std::multimap<VkDescriptorSetLayout, VkDescriptorSet> layout_and_set_map;

auto variable_descriptor(const uint32_t binding_less_size,
                         const std::vector<VkDescriptorBindingFlags> &binding_flags,
                         std::vector<uint32_t> &variableDescCounts) {
    ; // 也应该从一个 vector 传递过来， 然后再根据双缓冲进行翻倍
    //  variableDescCount 中的值如果是零的话，不能访问图片，如果是1 的话，实际上是退化为普通的
    // for (auto &variableDescCount: variableDescCounts) {
    //     variableDescCount = binding_less_size;
    // }
    variableDescCounts.resize(binding_flags.size());

    for (int i = 0; i < binding_flags.size(); i++) {
        if (binding_flags[i] & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
            variableDescCounts[i] = binding_less_size;
        } else {
            variableDescCounts[i] = 0;
        }
    }
    // Vulkan 协议强制规定：只有索引号（Binding Number）最大的那一个绑定可以是可变的
    // 位置限制： 只有描述符集布局中 Binding 编号最大 的那个绑定才能设置为可变长度。
    // 上限约束： 你在 pDescriptorCounts 中指定的数值，不能超过你在 VkDescriptorSetLayoutBinding 中定义的 descriptorCount（即最大上限）。
    // 特性开启： 需要在物理设备特性中开启 descriptorIndexing 的相关支持，具体可参考 Vulkan 硬件数据库 检查你的显卡是否支持 runtimeDescriptorArray
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
        .descriptorSetCount = static_cast<uint32_t>(variableDescCounts.size()),
        .pDescriptorCounts  = variableDescCounts.data(),
    };
    return variableDescCountAI;
}


Proxy_descriptor_sets allocate_descriptor_sets(const VkDescriptorPool &descriptorPool,
                                               std::vector<VkDescriptorSetLayout> descriptor_set_layouts,
                                               const std::vector<VkDescriptorBindingFlags> &binding_flags) {
    auto &backend = VK_backend::instance();
    Proxy_descriptor_sets return_value;
    std::vector<VkDescriptorSet> descriptor_sets;
    assert(!descriptor_set_layouts.empty()); // 因为不想多加一个判断的参数，强制 descriptor_set_layouts 不能为空
    descriptor_sets.resize(descriptor_set_layouts.size(),VK_NULL_HANDLE);
    return_value.resize(descriptor_set_layouts.size());

    bool need_allocate = false;
    for (int i = 0; i < descriptor_set_layouts.size(); i++) {
        auto it = layout_and_set_map.find(descriptor_set_layouts.at(i));
        if (it != layout_and_set_map.end()) {
            descriptor_sets.at(i) = it->second;
            return_value[i]       = std::make_shared<DescriptorSet_detail>(descriptor_sets.at(i),
                                                                     descriptor_set_layouts.at(i));
            descriptor_set_layouts.at(i) = VK_NULL_HANDLE;
            it                           = layout_and_set_map.erase(it);
            continue;
        }
        need_allocate = true;
        break;
    }
    if (need_allocate == false) {
        return return_value;
        // 如果失败的话，return_value 中的值会被重新覆盖, share_ptr 会自己 析构
        // 又重新会到了哪个问题，析构函数的 执行时机的问题 ，因为没有使用，所以没有问题
    }
    std::vector<uint32_t> variableDescCount;
    VkDescriptorSetAllocateInfo texDescSetAlloc{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(descriptor_set_layouts.size()), // // 打算分配的集合数量
        .pSetLayouts        = descriptor_set_layouts.data(), // 指向布局数组的指针,长度必须等于 descriptorSetCount
    };
    texDescSetAlloc.pNext = nullptr;
    if (!binding_flags.empty()) {
        for (const auto flag: binding_flags) {
            if (flag != 0) {
                const uint32_t binding_less_size = 512; // 这里肯定还是有问题的
                auto variableDescCountAI = variable_descriptor(binding_less_size, binding_flags, variableDescCount);
                texDescSetAlloc.pNext = &variableDescCountAI;
            }
        }
    }
    auto result = vkAllocateDescriptorSets(backend.get_device(), &texDescSetAlloc,
                                           descriptor_sets.data());
    if (result != VK_SUCCESS) {
        return {};
    }
    // 原来会黑屏不是内存的问题，是 vkAllocateDescriptorSets 申请不出来了
    for (uint32_t i = 0; i < descriptor_sets.size(); i++) {
        return_value[i] = std::make_shared<DescriptorSet_detail>(descriptor_sets.at(i),
                                                                 descriptor_set_layouts.at(i));
    }
    return return_value;
}


DescriptorSet_detail::~DescriptorSet_detail() {
    std::lock_guard<std::mutex> lock(discard_descriptor_set_map_mutex);
    discard_descriptor_set_map[descriptor_set_] = timeline_;
    layout_and_set_map.insert({descriptor_layout_, descriptor_set_});
    descriptor_set_    = VK_NULL_HANDLE;
    descriptor_layout_ = VK_NULL_HANDLE;
    timeline_          = 0;
}


void discard_descriptor_set_map_clean(uint64_t current_timeline) {
    // auto &backend = VK_backend::get();
    // for (auto it = discard_descriptor_set_map.begin(); it != discard_descriptor_set_map.end(); /* 后面不加 ++ */) {
    //     const auto &[descriptor_set, timeline] = *it;
    //     LOG_DEBUG(g_log(), "descriptor_pool finished timeline {}  , timeline {} ",
    //               current_timeline,
    //               timeline);
    //     if (current_timeline >= timeline) {
    //         std::lock_guard<std::mutex> lock(discard_descriptor_set_map_mutex);
    //         vkFreeDescriptorSets(backend.get_device(), Engine::get().get_descriptor_pool(), 1, &descriptor_set);
    //         原本这里 free 的时候是需要 descriptor_pool 这个参数的，但是呢？
    //         已经不需要 free 了，那么就可以不用这个参数了
    //         // vkDestroyDescriptorPool(handle.get_device(), descriptor_pool, nullptr);
    //         it = discard_descriptor_set_map.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
}

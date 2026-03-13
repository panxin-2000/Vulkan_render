//
// Created by 潘鑫 on 2026/3/4.
//

#include "vulkan_backend.h"
#include "pipeline_layout.h"

std::map<std::string, std::pair<VkPipelineLayout, uint32_t> > pipeline_layout_map_;


auto &get_pipeline_layout_map() {
    return pipeline_layout_map_;
}

std::vector<VkPushConstantRange> merge_push_constant_offset(
    std::map<std::string, VkPushConstantRange> &push_constant_map) {
    // 得到按照 offset 排列的 map
    std::map<uint32_t, VkPushConstantRange> sort_offset;
    for (const auto &[key, value]: push_constant_map) {
        sort_offset.insert({value.offset, value});
    }

    // 得到按照 offset 排列的 vector   // 其实可以直接掉用 sort 函数的
    std::vector<VkPushConstantRange> all_offset_vector;
    all_offset_vector.reserve(push_constant_map.size());
    for (const auto &[key, value]: sort_offset) {
        all_offset_vector.push_back(value);
    }


    // 合并连续的且内容相同的两个元素
    if (!all_offset_vector.empty())
        for (int i = 0; i < all_offset_vector.size() - 1; i++) {
            if (all_offset_vector[i].stageFlags == all_offset_vector[i + 1].stageFlags &&
                all_offset_vector[i].offset + all_offset_vector[i].size == all_offset_vector[i + 1].offset) {
                // stageFlags 已经相同，没有必要重复一遍
                all_offset_vector[i + 1].offset = all_offset_vector[i].offset;
                all_offset_vector[i + 1].size   = all_offset_vector[i].size + all_offset_vector[i + 1].size;
                all_offset_vector[i].offset     = 0;
                all_offset_vector[i].size       = 0;
                all_offset_vector[i].stageFlags = 0;
            }
        }

    // 得到合并后的 vector
    std::vector<VkPushConstantRange> merged_vector;
    all_offset_vector.reserve(push_constant_map.size());

    for (int i = 0; i < all_offset_vector.size(); i++) {
        if (all_offset_vector[i].size != 0 &&
            all_offset_vector[i].stageFlags != 0) {
            merged_vector.push_back(all_offset_vector[i]);
        }
    }
    return merged_vector;
}


/**
 * pipeline_layout 只是一个接口
 * 创建管线描述符布局, set = 0 还是 set = 1 需要在这里设置，解析需要更靠前
 * organize_graphics_descriptor_set_layouts  是 organize_graphics_descriptor_set_layouts 作为参数
 * 经由 create_descriptor_set_layouts 得出的结果
 * @param handle
 * @param descriptor_sets_layout  layout(set = 0, binding = 0) layout(set = 1, binding = 0)
 * @return
 */
VkPipelineLayout create_pipeline_layout(VK_backend &handle, const std::string shader_key,
                                        std::vector<VkDescriptorSetLayout> descriptor_sets_layout,
                                        std::map<std::string, VkPushConstantRange> &push_constant_map) {
    std::map<std::string, std::pair<VkPipelineLayout, uint32_t> > &map = get_pipeline_layout_map();
    if (!shader_key.empty()) {
        auto it = map.find(shader_key);
        if (it != map.end()) {
            it->second.second++;
            return it->second.first;
        } else {
            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
            auto push_Constant_last          = merge_push_constant_offset(push_constant_map);
            VkPipelineLayoutCreateInfo pipelineLayoutCI{
                .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount         = static_cast<uint32_t>(descriptor_sets_layout.size()),
                .pSetLayouts            = descriptor_sets_layout.data(),
                .pushConstantRangeCount = static_cast<uint32_t>(push_Constant_last.size()),
                .pPushConstantRanges    = push_Constant_last.data()
            };
            // VkPipelineLayout 的本质是 “接口协议”（Interface Protocol）。
            // 它定义了 Shader 如何访问资源（比如有哪些 Set，每个 Set 有哪些 Binding）。
            VK_CHECK_RESULT_NOT_EXIT(vkCreatePipelineLayout(handle.get_device(), &pipelineLayoutCI, nullptr, &
                                         pipeline_layout));
            if (pipeline_layout != VK_NULL_HANDLE) {
                map.insert({shader_key, {pipeline_layout, 1}});
            }
            return pipeline_layout;
        }
    }
    return VK_NULL_HANDLE;
}


VkPipelineLayout find_pipeline_layout(VK_backend &handle, const std::string shader_key) {
    std::map<std::string, std::pair<VkPipelineLayout, uint32_t> > &map = get_pipeline_layout_map();
    if (!shader_key.empty()) {
        auto it = map.find(shader_key);
        if (it != map.end()) {
            return it->second.first;
        } else {
            return nullptr;
        }
    }
}

void clean_all_pipeline_layout(VK_backend &handle) {
    auto &map = get_pipeline_layout_map();
    for (const auto &[key, value]: map) {
        vkDestroyPipelineLayout(handle.get_device(), value.first, nullptr);
    }
    map.clear();
}

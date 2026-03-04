//
// Created by 潘鑫 on 2026/3/4.
//

#include "sets_and_bindings_layout.h"

std::map<std::string, std::pair<std::vector<VkDescriptorSetLayout>, uint32_t> > descriptor_sets_layout_map_;

auto &get_descriptor_sets_layout_map() {
    return descriptor_sets_layout_map_;
}


VkDescriptorSetLayoutBindingFlagsCreateInfo DescriptorSetLayoutBindingFlagsCreateInfo(
    const std::vector<VkDescriptorBindingFlags> &descVariableFlags) {
    const VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount  = static_cast<uint32_t>(descVariableFlags.size()),
        .pBindingFlags = descVariableFlags.data(),
    };
    return descBindingFlags;
}


/**
*  输出的结果是 bindings
 * 举例如 binding = 0 和 binding =1 等等的组合
 * 两个参数的 vector 的 size 需要一致
 * @param handle
 * @param layout_bindings        输入的是 layout
 * @param layout_bindings_flags  哪怕 全部填零也是需要一致的
 * @return 输出的是 descriptor
 */
VkDescriptorSetLayout
create_descriptor_bindings_layout(const VK_handle &handle,
                                  const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                                  const std::vector<VkDescriptorBindingFlags> &layout_bindings_flags) {
    VkDescriptorSetLayout descriptor_bindings_layout;

    // descVariableFlags 要么没有，要么需要和 setLayoutBindings 一致
    const auto descriptor_bindings_flags = DescriptorSetLayoutBindingFlagsCreateInfo(layout_bindings_flags);

    const auto descriptorLayout = descriptorSetLayoutCreateInfo(layout_bindings,
                                                                (void *) &descriptor_bindings_flags,
                                                                layout_bindings_flags);
    VK_CHECK_RESULT_NOT_EXIT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
                                 descriptor_bindings_layout));
    return descriptor_bindings_layout;
}

/**
 * 输出的结果是 sets_layout
 * 举例如 set = 0 和 set =1 等等的组合
 * @param handle
 * @param organized_sets_and_bindings
 * @return
 */
std::vector<VkDescriptorSetLayout> create_descriptor_sets_layout(VK_handle &handle,
                                                                 const std::string &shader_key,
                                                                 const sets_map &organized_sets_and_bindings) {
    std::vector<VkDescriptorSetLayout> descriptor_sets_layout;
    if (!shader_key.empty()) {
        auto &map = get_descriptor_sets_layout_map();
        auto it   = map.find(shader_key);
        if (it != map.end()) {
            it->second.second++;
            return it->second.first;
        } else {
            for (auto const &[set_value, bindings_map]: organized_sets_and_bindings) {
                const auto &organized_bindings = bindings_map;
                if (organized_bindings.empty() == true)
                    continue; // 我不确定，
                std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
                std::vector<VkDescriptorBindingFlags> layout_bindings_flags;
                for (const auto &[fst, snd]: organized_bindings) {
                    layout_bindings.push_back(snd.LayoutBinding);
                    layout_bindings_flags.push_back(snd.flag);
                }
                if (layout_bindings_flags.empty() == true && layout_bindings.empty() == true) {
                    continue;
                }
                auto set_x_layout = create_descriptor_bindings_layout(handle, layout_bindings, layout_bindings_flags);
                descriptor_sets_layout.push_back(set_x_layout);
            }
            if (!descriptor_sets_layout.empty()) {
                map.insert({shader_key, {descriptor_sets_layout, 1}});
            }
            return descriptor_sets_layout;
        }
    }
    return descriptor_sets_layout;
}

std::vector<VkDescriptorSetLayout> find_descriptor_sets_layout(VK_handle &handle,
                                                               const std::string &shader_key) {
    std::vector<VkDescriptorSetLayout> descriptor_sets_layout;
    if (!shader_key.empty()) {
        auto &map = get_descriptor_sets_layout_map();
        auto it   = map.find(shader_key);
        if (it != map.end()) {
            it->second.second++;
            return it->second.first;
        } else {
        }
    }
    return descriptor_sets_layout;
}


void clean_all_descriptor_sets_layout(VK_handle &handle) {
    auto &map = get_descriptor_sets_layout_map();
    for (const auto &[key, value]: map) {
        for (auto Bindings: value.first) {
            vkDestroyDescriptorSetLayout(handle.get_device(), Bindings, nullptr);
        }
    }
    map.clear();
}

std::vector<VkDescriptorBindingFlags> create_descriptor_sets_flags(const VK_handle &handle,
                                                                   const sets_map &organized_sets_and_bindings) {
    std::vector<VkDescriptorBindingFlags> sets_flags;
    for (auto const &[set_value, bindings_map]: organized_sets_and_bindings) {
        const auto &organized_bindings = bindings_map;
        std::vector<VkDescriptorBindingFlags> layout_bindings_flags;
        VkDescriptorBindingFlags set_x_layout_binding_flags = 0;
        for (const auto &[fst, snd]: organized_bindings) {
            layout_bindings_flags.push_back(snd.flag);
            set_x_layout_binding_flags = set_x_layout_binding_flags | snd.flag;
        }
        if (layout_bindings_flags.empty() == true) {
            continue;
        }
        sets_flags.push_back(set_x_layout_binding_flags);
    }
    return sets_flags;
}

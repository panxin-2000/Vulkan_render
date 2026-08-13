//
// Created by 潘鑫 on 2026/2/9.
//

#ifndef HELLO_MAC_SETS_AND_BINDINGS_LAYOUT_H
#define HELLO_MAC_SETS_AND_BINDINGS_LAYOUT_H

#include "vulkan_backend.h"
#include "descriptor_organized_sets_and_bindings.h"


VkDescriptorSetLayoutBindingFlagsCreateInfo DescriptorSetLayoutBindingFlagsCreateInfo(
    const std::vector<VkDescriptorBindingFlags> &descVariableFlags);


/**
*  输出的结果是 bindings
 * 举例如 binding = 0 和 binding =1 等等的组合
 * 两个参数的 vector 的 size 需要一致
 * @param backend
 * @param layout_bindings        输入的是 layout
 * @param layout_bindings_flags  哪怕 全部填零也是需要一致的
 * @return 输出的是 descriptor
 */
VkDescriptorSetLayout
create_descriptor_bindings_layout(const VK_backend &backend,
                                  const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                                  const std::vector<VkDescriptorBindingFlags> &layout_bindings_flags);

/**
 * 输出的结果是 sets_layout
 * 举例如 set = 0 和 set =1 等等的组合
 * @param handle
 * @param organized_sets_and_bindings
 * @return
 */
std::vector<VkDescriptorSetLayout> create_descriptor_sets_layout(VK_backend &handle,
                                                                 const std::string &shader_key,
                                                                 const sets_map &organized_sets_and_bindings);

std::vector<VkDescriptorBindingFlags> create_descriptor_sets_flags(const VK_backend &backend,
                                                                   const sets_map &organized_sets_and_bindings);


#endif //HELLO_MAC_SETS_AND_BINDINGS_LAYOUT_H

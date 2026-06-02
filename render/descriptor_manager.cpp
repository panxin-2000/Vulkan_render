//
// Created by 潘鑫 on 2026/6/2.
//

#include "descriptor_manager.h"

std::map<VkDescriptorSet, uint64_t> discard_descriptor_set_map;
std::mutex discard_descriptor_set_map_mutex;

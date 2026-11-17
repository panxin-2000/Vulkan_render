//
// Created by 潘鑫 on 2026/2/15.
//

#ifndef HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H
#define HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H
#include "vulkan_buffer.h"

#include <algorithm>
VKR_buffer_pool_ptr &get_uniform_buffer();

VKR_buffer_ptr create_SSBO_buffer(VkDeviceSize size);


VKR_buffer_ptr create_SSBO_buffer(VkDeviceSize size, const void *src);


template<typename T1>
VKR_buffer_ptr copy_data_to_SSBO_buffer(const std::vector<T1> &data) {
    const auto primitives_ptr = data.data();
    auto primitives_size      = data.size() * sizeof(T1);
    auto primitives_buffer    = create_SSBO_buffer(primitives_size, primitives_ptr);
    return primitives_buffer;
}


#endif //HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H

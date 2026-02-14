//
// Created by 潘鑫 on 2026/2/15.
//

#ifndef HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H
#define HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H


#include "vulkan_device_handle.h"

uniform_buffer &get_uniform_buffer();

template<typename... Args>
VkDeviceAddress update_push_constants_data(Args... args) {
    auto &buffer         = get_uniform_buffer();
    uint32_t memory_size = 0;
    ([&] {
        memory_size += sizeof(args);
    }(), ...);
    // 从内存中分配
    const auto offset_of_start_address = buffer.alloc_size(memory_size);
    auto start_address                 = buffer.get_point_mapped_address();
    if (offset_of_start_address != -1) {
        start_address          = static_cast<char *>(start_address) + offset_of_start_address;
        uint64_t memory_offset = 0;
        ([&] {
            std::copy_n(reinterpret_cast<const char *>(&args), sizeof(args),
                        static_cast<char *>(start_address) + memory_offset);
            memory_offset += sizeof(args);
        }(), ...);
        return buffer.get_gpu_device_address() + offset_of_start_address;
    } else {
        return 0;
    }
}


#endif //HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H

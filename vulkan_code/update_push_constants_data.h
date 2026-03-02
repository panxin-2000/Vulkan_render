//
// Created by 潘鑫 on 2026/2/15.
//

#ifndef HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H
#define HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H


#include "vulkan_device_handle.h"

VKR_buffer_pool &get_uniform_buffer();

template<typename... Args>
VKR_buffer_block_ptr copy_data_to_gpu_buffer(Args... args) {
    auto &buffer         = get_uniform_buffer();
    uint32_t memory_size = 0;
    ([&] {
        memory_size += sizeof(args);
    }(), ...);
    // 从内存中分配
    const auto offset_address = buffer.alloc_size(memory_size);
    auto buffer_start_address = buffer->mapped_address();
    if (offset_address != -1) {
        buffer_start_address   = static_cast<char *>(buffer_start_address) + offset_address;
        uint64_t memory_offset = 0;
        ([&] {
            std::copy_n(reinterpret_cast<const char *>(&args), sizeof(args),
                        static_cast<char *>(buffer_start_address) + memory_offset);
            memory_offset += sizeof(args);
        }(), ...);
        return {static_cast<VKR_buffer_ptr>(buffer), offset_address, memory_size};
    } else {
        return {{}, 0, 0};
    }
}


#endif //HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H

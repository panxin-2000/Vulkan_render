//
// Created by 潘鑫 on 2026/2/15.
//

#ifndef HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H
#define HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H
#include "vulkan_buffer.h"


VKR_buffer_pool_ptr &get_uniform_buffer();

template<typename... Args>
VKR_buffer_block_ptr copy_data_to_gpu_buffer(Args... args) {
    const auto &buffer = get_uniform_buffer();

    uint32_t memory_size = 0;
    ([&] {
        memory_size += sizeof(args);
    }(), ...);
    // 从内存中分配
    const auto return_value = GPU_pool_alloc(buffer, memory_size);
    // auto complete_size        = return_value->complete_size();
    auto buffer_start_address = buffer->mapped_address();
    if (return_value) {
        buffer_start_address   = static_cast<char *>(buffer_start_address) + return_value->offset_;
        uint64_t memory_offset = 0;
        ([&] {
            std::copy_n(reinterpret_cast<const char *>(&args), sizeof(args),
                        static_cast<char *>(buffer_start_address) + memory_offset);
            memory_offset += sizeof(args);
        }(), ...);
        return return_value;
    } else {
        return {};
    }
}


#endif //HELLO_MAC_UPDATE_PUSH_CONSTANTS_DATA_H

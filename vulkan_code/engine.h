//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H
#include <list>

#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>

#include "model_matrix.h"


struct ShaderData {
    matrix_4x4 projection;
    matrix_4x4 view;
    matrix_4x4 model[3];
    float lightPos[4]{0.0f, -10.0f, 10.0f, 0.0f};
    uint32_t selected{1};
    uint32_t selected8{1};
    uint32_t selected7{1};
    uint32_t selected6{1};
};

struct address_and_length {
    uint64_t address = 0;
    uint64_t length  = 0;
    bool if_used     = false;
};

struct uniform_buffer {
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};


    [[nodiscard]] void *get_point_mapped_address() const;

    [[nodiscard]] VkDeviceAddress get_gpu_device_address() const;

    // 上面的可以作为一个单独的结构体，下面的内容只是和分配有关的内容
    std::list<address_and_length> memory_pool;

    uint64_t alloc_size(const uint64_t size) {
        uint64_t return_address = -1;
        for (auto it = memory_pool.begin(); it != memory_pool.end(); ++it) {
            if (it->if_used == false && it->length == size) {
                it->if_used    = true;
                return_address = it->address;
                break;
            }
            if (it->if_used == false && it->length > size) {
                memory_pool.insert(it, address_and_length{it->address, size, true});
                return_address = it->address;
                it->address    += size;
                it->length     -= size;
                break;
            }
        }
        return return_address;
    }
};


#endif //HELLO_MAC_ENGINE_H

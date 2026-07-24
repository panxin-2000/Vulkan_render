//
// Created by 潘鑫 on 2026/7/24.
//

#ifndef HELLO_MAC_PBR_MANAGER_H
#define HELLO_MAC_PBR_MANAGER_H
#include "PBR_component.h"
#include "vulkan_buffer.h"


class PBR_manager {
    std::atomic<uint32_t> pbr_components_size_;
    std::vector<PBR_component> pbr_components_;
    std::vector<PBR_Texture_ptr> pbr_texture_ptr_;
    moodycamel::BlockingReaderWriterQueue<uint32_t> free_index;
    std::atomic<uint32_t> max_index;

public:
    PBR_manager() {
    }

    auto data() {
        return pbr_components_.data();
    }


    size_t size() const {
        return max_index.load();
    }

    uint32_t push(const PBR_component &component, const PBR_Texture_ptr &texture) {
        uint32_t return_value;
        if (free_index.try_dequeue(return_value) == true) {
        } else {
            return_value = max_index.load();
            ++max_index;
        }
        pbr_components_.resize(max_index);
        pbr_texture_ptr_.resize(max_index);
        pbr_components_[return_value]  = component;
        pbr_texture_ptr_[return_value] = texture;
        return return_value;
    }


    bool free(const uint32_t index) {
        free_index.enqueue(index);
        return true;
    }

    void destroy() {
        pbr_components_.clear();
        pbr_texture_ptr_.clear();
    }
};


#endif //HELLO_MAC_PBR_MANAGER_H

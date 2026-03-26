//
// Created by 潘鑫 on 2026/3/27.
//

#ifndef HELLO_MAC_VULKAN_TEXTURE_BINDLESS_H
#define HELLO_MAC_VULKAN_TEXTURE_BINDLESS_H

#include <map>
#include <string>

#include "vulkan_image.h"
#include "vulkan_update_descriptor.h"

uint32_t free_bindless_uniform_sampler2D(const std::string &name);

uint32_t add_bindless_uniform_sampler2D(const std::string &name,
                                        std::optional<Texture_parameter> &update);


class bindless_uniform_sampler2D {
public:
    std::map<std::string, std::pair<uint32_t, Update_descriptor_binding> > bindings;
    std::queue<uint32_t> freeSlots;
};

uint32_t free_bindless_uniform_sampler2D(const std::string &name, bindless_uniform_sampler2D &bindless);


#endif //HELLO_MAC_VULKAN_TEXTURE_BINDLESS_H

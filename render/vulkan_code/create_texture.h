//
// Created by 潘鑫 on 2026/3/4.
//

#ifndef HELLO_MAC_CREATE_TEXTURE_H
#define HELLO_MAC_CREATE_TEXTURE_H
#include <map>
#include "shader_common.h"

std::map<std::string, texture_and_share> &get_texture_map();

std::vector<VkDescriptorImageInfo> &get_bindless_textures();
#endif //HELLO_MAC_CREATE_TEXTURE_H

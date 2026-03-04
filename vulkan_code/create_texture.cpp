//
// Created by 潘鑫 on 2026/3/4.
//

#include "create_texture.h"

#include <map>

#include "shader_common.h"

std::map<std::string, texture_and_share> texture_map_;

auto &get_texture_map() {
    return texture_map_;
}

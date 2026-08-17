//
// Created by 潘鑫 on 2026/8/16.
//


#include "Shader_manager.h"
#include "shader_resolve.h"


void Shader_manager::destroy() {
    map_.clear();
}


std::shared_ptr<vk_shader_data> Shader_manager::get_gltf_shader_data() {
    return find(VKR_shader_paths{"pbr_bindless", "pbr_bindless", "", ""});
}

std::shared_ptr<vk_shader_data> Shader_manager::get_skinning_shader_data() {
    return find(VKR_shader_paths{"skinning_model", "pbr_bindless", "", ""});
}

std::shared_ptr<vk_shader_data> Shader_manager::get_line_shader_data() {
    return find(VKR_shader_paths{"line", "line", "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST});
}

std::shared_ptr<vk_shader_data> Shader_manager::get_frustum_cull_shader_data() {
    return find(VKR_shader_paths{"", "", "", "command_calculate"});
}

std::shared_ptr<vk_shader_data> Shader_manager::get_offscreen_to_screen_shader_data() {
    return find(VKR_shader_paths{"deferred", "deferred_to_screen", "", ""});
}

std::shared_ptr<vk_shader_data> Shader_manager::find(const VKR_shader_paths &shader_paths) {
    if (map_.contains(shader_paths)) {
        return map_[shader_paths];
    } else {
        auto temp          = VKR_shader_init(shader_paths);
        map_[shader_paths] = temp;
        return temp;
    }
}

void Shader_manager::create() {
    find(VKR_shader_paths{"pbr_bindless", "pbr_bindless", "", ""});
    find(VKR_shader_paths{"opacity_depth_write", "opacity_depth_write", "", ""});
    find(VKR_shader_paths{"skinning_model_depth_write", "opacity_depth_write", "", ""});
    find(VKR_shader_paths{"skinning_model", "pbr_bindless", "", ""});
    find(VKR_shader_paths{"line", "line", "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST});
    find(VKR_shader_paths{"", "", "", "command_calculate"});
    find(VKR_shader_paths{"deferred", "fxaa", "", ""});
    find(VKR_shader_paths{"deferred", "deferred_to_screen", "", ""});

}

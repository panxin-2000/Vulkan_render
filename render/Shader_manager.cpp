//
// Created by 潘鑫 on 2026/8/16.
//


#include "Shader_manager.h"
#include "shader_resolve.h"


void Shader_manager::destroy() {
    map_.clear();
}

VKR_shader_paths get_gltf_shader_path() {
    return VKR_shader_paths{
        "pbr_bindless", "pbr_bindless", "", "",
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_UNDEFINED,
        VKR_shader_paths::Render_Pass_Type::Color
    };
}

VKR_shader_paths get_skinning_shader_path() {
    return VKR_shader_paths{
        "skinning_model", "pbr_bindless", "", "",
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_UNDEFINED,
        VKR_shader_paths::Render_Pass_Type::Color
    };
}

VKR_shader_paths Shader_manager::get_line_shader_path() {
    return VKR_shader_paths{
        "2D/line", "2D/line", "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST
    };
}

std::shared_ptr<vk_shader_data> Shader_manager::get_gltf_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(get_gltf_shader_path());
}

std::shared_ptr<vk_shader_data> Shader_manager::get_skinning_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(get_skinning_shader_path());
}

std::shared_ptr<vk_shader_data> Shader_manager::get_line_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(get_line_shader_path());
}

std::shared_ptr<vk_shader_data> Shader_manager::get_frustum_cull_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "", "", "", "frustum_cull",
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FORMAT_UNDEFINED,
                    VK_FORMAT_UNDEFINED,
                    render_pass_type
                });
}

std::shared_ptr<vk_shader_data> Shader_manager::get_offscreen_to_screen_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "full_screen_triangle", "fxaa", "", "", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FORMAT_UNDEFINED,
                    VK_FORMAT_UNDEFINED, render_pass_type
                });
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
}

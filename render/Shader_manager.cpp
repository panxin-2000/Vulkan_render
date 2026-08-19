//
// Created by 潘鑫 on 2026/8/16.
//


#include "Shader_manager.h"
#include "shader_resolve.h"


void Shader_manager::destroy() {
    map_.clear();
}


std::shared_ptr<vk_shader_data> Shader_manager::get_gltf_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "pbr_bindless", "pbr_bindless", "", "",
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FORMAT_UNDEFINED,
                    VK_FORMAT_UNDEFINED,
                    render_pass_type
                });
}

std::shared_ptr<vk_shader_data> Shader_manager::get_skinning_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "skinning_model", "pbr_bindless", "", "",
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FORMAT_UNDEFINED,
                    VK_FORMAT_UNDEFINED, render_pass_type
                });
}

std::shared_ptr<vk_shader_data> Shader_manager::get_line_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "line", "line", "", "",
                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                    VK_FORMAT_UNDEFINED,
                    VK_FORMAT_UNDEFINED,
                    render_pass_type
                });
}

std::shared_ptr<vk_shader_data> Shader_manager::get_frustum_cull_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "", "", "", "command_calculate",
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FORMAT_UNDEFINED,
                    VK_FORMAT_UNDEFINED,
                    render_pass_type
                });
}

std::shared_ptr<vk_shader_data> Shader_manager::get_offscreen_to_screen_shader_data(
    VKR_shader_paths::Render_Pass_Type render_pass_type) {
    return find(VKR_shader_paths{
                    "deferred", "fxaa", "", "", VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FORMAT_UNDEFINED,
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
    // find(VKR_shader_paths{"pbr_bindless", "pbr_bindless", "", ""});
    // find(VKR_shader_paths{
    //          "opacity_depth_write", "opacity_depth_write", "", "",
    //          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    //          VK_FORMAT_D32_SFLOAT
    //      });
    // find(VKR_shader_paths{
    //          "skinning_model_depth_write", "opacity_depth_write", "", "",
    //          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    //          VK_FORMAT_D32_SFLOAT
    //      });
    // find(VKR_shader_paths{"skinning_model", "pbr_bindless", "", ""});
    // find(VKR_shader_paths{"line", "line", "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST});
    // find(VKR_shader_paths{"", "", "", "command_calculate"});
    // find(VKR_shader_paths{"deferred", "fxaa", "", ""});
    // find(VKR_shader_paths{"deferred", "deferred_to_screen", "", ""});
}

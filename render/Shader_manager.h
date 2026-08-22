//
// Created by 潘鑫 on 2026/8/15.
//

#ifndef HELLO_MAC_SHADER_MANAGER_H
#define HELLO_MAC_SHADER_MANAGER_H

#include "shader_resolve.h"
#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"

class Shader_manager {
    absl::flat_hash_map<VKR_shader_paths, std::shared_ptr<vk_shader_data> > map_;

public:
    VKR_shader_paths get_line_shader_path();

    std::shared_ptr<vk_shader_data> get_gltf_shader_data(
        VKR_shader_paths::Render_Pass_Type render_pass_type = VKR_shader_paths::Render_Pass_Type::Color);

    std::shared_ptr<vk_shader_data> get_skinning_shader_data(
        VKR_shader_paths::Render_Pass_Type render_pass_type = VKR_shader_paths::Render_Pass_Type::Color);

    std::shared_ptr<vk_shader_data> get_line_shader_data(
        VKR_shader_paths::Render_Pass_Type render_pass_type = VKR_shader_paths::Render_Pass_Type::Color);

    std::shared_ptr<vk_shader_data> get_frustum_cull_shader_data(
        VKR_shader_paths::Render_Pass_Type render_pass_type = VKR_shader_paths::Render_Pass_Type::Color);

    std::shared_ptr<vk_shader_data> get_offscreen_to_screen_shader_data(
        VKR_shader_paths::Render_Pass_Type render_pass_type = VKR_shader_paths::Render_Pass_Type::Color);

    std::shared_ptr<vk_shader_data> find(const VKR_shader_paths &shader_paths);

    void create();

    void destroy();
};


#endif //HELLO_MAC_SHADER_MANAGER_H

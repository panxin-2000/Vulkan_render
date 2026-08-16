//
// Created by 潘鑫 on 2026/8/15.
//

#ifndef HELLO_MAC_SHADER_MANAGER_H
#define HELLO_MAC_SHADER_MANAGER_H

#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"

class Shader_manager {
    std::shared_ptr<vk_shader_data> gltf_shader_data;
    std::shared_ptr<vk_shader_data> gltf_shader_opacity_data;
    std::shared_ptr<vk_shader_data> skinning_date;
    std::shared_ptr<vk_shader_data> skinning_opacity_date;
    std::shared_ptr<vk_shader_data> line_date;
    std::shared_ptr<vk_shader_data> frustum_cull;
    std::shared_ptr<vk_shader_data> offscreen_to_screen;
    std::shared_ptr<vk_shader_data> bindless_shader_date;

    absl::flat_hash_map<VKR_shader_paths, std::shared_ptr<vk_shader_data> > map_;

public:
    std::shared_ptr<vk_shader_data> get_gltf_shader_data() {
        return gltf_shader_data;
    }

    std::shared_ptr<vk_shader_data> get_skinning_shader_data() {
        return skinning_date;
    }

    std::shared_ptr<vk_shader_data> get_line_shader_data() {
        return line_date;
    }

    std::shared_ptr<vk_shader_data> get_frustum_cull_shader_data() {
        return frustum_cull;
    }

    std::shared_ptr<vk_shader_data> get_offscreen_to_screen_shader_data() {
        return offscreen_to_screen;
    }


    void create() { {
            VKR_shader_paths shader_paths{
                "pbr_bindless",
                "pbr_bindless",
                "", ""
            };
            gltf_shader_data   = VKR_shader_init(shader_paths);
            map_[shader_paths] = gltf_shader_data;
        } {
            VKR_shader_paths shader_paths{
                "opacity_depth_write",
                "opacity_depth_write",
                "", ""
            };
            gltf_shader_opacity_data = VKR_shader_init(shader_paths);
            map_[shader_paths]       = gltf_shader_opacity_data;
        } {
            VKR_shader_paths shader_paths{
                "skinning_model_depth_write",
                "opacity_depth_write",
                "", ""
            };
            skinning_opacity_date = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "skinning_model",
                "pbr_bindless",
                "", ""
            };
            skinning_date = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "line",
                "line",
                "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST
            };
            line_date = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "",
                "",
                "",
                "command_calculate"
            };
            frustum_cull = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "deferred",
                "fxaa",
                "",
                ""
            };

            offscreen_to_screen = VKR_shader_init(shader_paths);
        }
    }

    void destroy() {
        gltf_shader_data         = nullptr;
        skinning_date            = nullptr;
        gltf_shader_opacity_data = nullptr;
        skinning_opacity_date    = nullptr;
        line_date                = nullptr;
        frustum_cull             = nullptr;
        offscreen_to_screen      = nullptr;
        bindless_shader_date     = nullptr;
    }
};

#endif //HELLO_MAC_SHADER_MANAGER_H

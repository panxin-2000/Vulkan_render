//
// Created by 潘鑫 on 2026/8/15.
//

#ifndef HELLO_MAC_SHADER_MANAGER_H
#define HELLO_MAC_SHADER_MANAGER_H


class Shader_manager {
    std::shared_ptr<vk_shader_data> gltf_shader_data;
    std::shared_ptr<vk_shader_data> skinning_date;
    std::shared_ptr<vk_shader_data> line_date;
    std::shared_ptr<vk_shader_data> frustum_cull;
    std::shared_ptr<vk_shader_data> offscreen_to_screen;
    std::shared_ptr<vk_shader_data> bindless_shader_date;

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
                "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.vert.spv",
                "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
                "", ""
            };
            gltf_shader_data = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "/Users/panxin/CLionProjects/hello_mac/render/shader/skinning_model.vert.spv",
                "/Users/panxin/CLionProjects/hello_mac/render/shader/pbr_bindless.frag.spv",
                "", ""
            };
            skinning_date = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "/Users/panxin/CLionProjects/hello_mac/render/shader/line.vert.spv",
                "/Users/panxin/CLionProjects/hello_mac/render/shader/line.frag.spv",
                "", "", VK_PRIMITIVE_TOPOLOGY_LINE_LIST
            };
            line_date = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "",
                "",
                "",
                "/Users/panxin/CLionProjects/hello_mac/render/shader/command_calculate.comp.spv"
            };
            frustum_cull = VKR_shader_init(shader_paths);
        } {
            VKR_shader_paths shader_paths{
                "/Users/panxin/CLionProjects/hello_mac/render/shader/deferred.vert.spv",
                "/Users/panxin/CLionProjects/hello_mac/render/shader/fxaa.frag.spv",
                "",
                ""
            };

            offscreen_to_screen = VKR_shader_init(shader_paths);
        }
    }

    void destroy() {
        gltf_shader_data     = nullptr;
        skinning_date        = nullptr;
        line_date            = nullptr;
        frustum_cull         = nullptr;
        offscreen_to_screen  = nullptr;
        bindless_shader_date = nullptr;
    }
};

#endif //HELLO_MAC_SHADER_MANAGER_H

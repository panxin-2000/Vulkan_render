//
// Created by 潘鑫 on 2026/8/3.
//

#include "skybox.h"

#include "base_3d_render_object.h"
#include "shader_component.h"
#include "vulkan_image.h"


void add_skybox_entity() {
    {
        std::vector<std::string> paths;
        paths.emplace_back("assets/skybox_right.jpg");
        paths.emplace_back("assets/skybox_left.jpg");
        paths.emplace_back("assets/skybox_top.jpg");
        paths.emplace_back("assets/skybox_bottom.jpg");
        paths.emplace_back("assets/skybox_front.jpg");
        paths.emplace_back("assets/skybox_back.jpg");

        auto texture                                    = create_skybox_texture_all(paths);
        std::optional<Texture_parameter> sampler_skybox = texture;
        create_object_3d("skybox")
                .add_sky_box()
                .add_render_parameter("sampler_skybox", sampler_skybox);
    }
}

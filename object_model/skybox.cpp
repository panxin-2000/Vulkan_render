//
// Created by 潘鑫 on 2026/8/3.
//

#include "skybox.h"

#include "base_3d_render_object.h"
#include "shader_component.h"
#include "vulkan_image.h"


void add_skybox_entity() {
    {
        auto texture                                    = create_skybox_texture_all("");
        std::optional<Texture_parameter> sampler_skybox = texture;
        create_object_3d("skybox")
                .add_sky_box()
                .add_render_parameter("sampler_skybox", sampler_skybox);
    }
}

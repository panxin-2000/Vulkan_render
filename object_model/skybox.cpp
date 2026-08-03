//
// Created by 潘鑫 on 2026/8/3.
//

#include "skybox.h"

#include "3d_model_display.h"
#include "shader_component.h"
#include "vulkan_image.h"


void add_skybox_entity() {
    {
        auto entity                                     = add_sky_box("skybox");
        auto texture                                    = create_skybox_texture_all("");
        std::optional<Texture_parameter> sampler_skybox = texture;
        set_render_parameter(entity, "sampler_skybox", sampler_skybox);
        logic_update_add_tag<skybox_tag>(entity);
        // 还需再增加一个特殊的标记，用于最后绘制，UI前，所有3D 完成后
    }
}

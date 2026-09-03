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

        auto result = create_object_3d("skybox");

        result.add_shader_path(VKR_shader_paths{
                                   "skybox", "skybox", "", "",
                                   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                   VK_backend::instance().get_depth_format(),
                                   VK_backend::instance().get_depth_format(),
                                   VKR_shader_paths::Render_Pass_Type::Color
                               });
        result.add_sky_box();
        result.add_render_parameter("sampler_skybox", sampler_skybox);
    }
}

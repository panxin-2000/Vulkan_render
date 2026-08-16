//
// Created by 潘鑫 on 2026/8/15.
//

#ifndef HELLO_MAC_RENDER_IMAGE_MANAGER_H
#define HELLO_MAC_RENDER_IMAGE_MANAGER_H
#include "vulkan_backend.h"
#include "vulkan_image.h"


class Render_image_manager {
    std::vector<VKR_image_ptr> G_buffer_Position_images_;
    std::vector<VKR_image_ptr> g_buffer_Normal_images_;
    std::vector<VKR_image_ptr> G_buffer_BaseColor_images_;
    std::vector<VKR_image_ptr> depth_images_;

    Texture_parameter temp;
    Texture_parameter temp_depth;

public:
    VKR_image_ptr &get_one_position_image();

    VKR_image_ptr &get_one_normal_image();

    VKR_image_ptr &get_one_color_image();

    std::optional<Texture_parameter> get_color_texture();

    std::optional<Texture_parameter> get_depth_texture();

    void create();

    VKR_image_ptr get_one_depth_image();

    void destroy();
};

#endif //HELLO_MAC_RENDER_IMAGE_MANAGER_H

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
    Texture_parameter temp;

public:
    VKR_image_ptr &get_one_position_image() {
        return G_buffer_Position_images_.back();
    }

    VKR_image_ptr &get_one_normal_image() {
        return g_buffer_Normal_images_.back();
    }

    VKR_image_ptr &get_one_color_image() {
        return G_buffer_BaseColor_images_.back();
    }

    std::optional<Texture_parameter> get_color_texture() {
        return temp;
    }

    void create();


    void destroy() {
        for (const auto &image: G_buffer_Position_images_) {
            image->destroy_image();
        }
        G_buffer_Position_images_.clear();
        for (const auto &image: g_buffer_Normal_images_) {
            image->destroy_image();
        }
        g_buffer_Normal_images_.clear();
        for (const auto &image: G_buffer_BaseColor_images_) {
            image->destroy_image();
        }
        G_buffer_BaseColor_images_.clear();
    }
};

#endif //HELLO_MAC_RENDER_IMAGE_MANAGER_H

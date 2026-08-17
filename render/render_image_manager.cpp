//
// Created by 潘鑫 on 2026/8/15.
//
#include "render_image_manager.h"

#include "engine.h"

VKR_image_ptr &Render_image_manager::get_one_position_image() {
    return G_buffer_Position_images_.back();
}

VKR_image_ptr &Render_image_manager::get_one_normal_image() {
    return g_buffer_Normal_images_.back();
}

VKR_image_ptr &Render_image_manager::get_one_color_image() {
    return G_buffer_BaseColor_images_.back();
}

std::optional<Texture_parameter> Render_image_manager::get_color_texture() {
    return temp;
}

std::optional<Texture_parameter> Render_image_manager::get_depth_texture() {
    return temp_depth;
}

void Render_image_manager::create() {
    {
        Image_and_view_parameters parameters;

        parameters.format = VK_FORMAT_D32_SFLOAT;
        parameters.width  = 2016;
        parameters.height = 1832;
        parameters.depth  = 1;
        parameters.usage  = static_cast<VkImageUsageFlagBits>(
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        parameters.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        parameters.tiling     = VK_IMAGE_TILING_OPTIMAL;
        parameters.mipLevels  = 1;
        auto result           = create_2d_image_and_view(parameters);

        depth_images_.push_back(result);
        depth_images_.push_back(VK_backend::instance().create_depth_image_and_view());
        depth_images_.push_back(VK_backend::instance().create_depth_image_and_view());


        G_buffer_Position_images_.push_back(VK_backend::instance().
                                            create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_Position_images_.push_back(VK_backend::instance().
                                            create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        g_buffer_Normal_images_.push_back(VK_backend::instance().
                                          create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        g_buffer_Normal_images_.push_back(VK_backend::instance().
                                          create_G_buffer_image_and_view(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_BaseColor_images_.push_back(VK_backend::instance().
                                             create_G_buffer_image_and_view(VK_FORMAT_B8G8R8A8_SRGB,
                                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        G_buffer_BaseColor_images_.push_back(VK_backend::instance().
                                             create_G_buffer_image_and_view(VK_FORMAT_B8G8R8A8_SRGB,
                                                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
        temp       = create_2d_texture(G_buffer_BaseColor_images_.back());
        temp_depth = create_2d_texture(depth_images_.at(0));
    }
}

VKR_image_ptr Render_image_manager::get_one_depth_image() {
    return depth_images_.back();
}

VKR_image_ptr Render_image_manager::get_one_depth_AO_image() {
    return depth_images_.at(0);
}

void Render_image_manager::destroy() {
    for (const auto &image: depth_images_) {
        image->destroy_image();
    }
    depth_images_.clear();

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

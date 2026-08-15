//
// Created by 潘鑫 on 2026/8/15.
//
#include "render_image_manager.h"

#include "engine.h"

void Render_image_manager::create() {
    {

        depth_images_.push_back(VK_backend::instance().create_depth_image_and_view());
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
        temp = create_2d_texture(G_buffer_BaseColor_images_.back());
    }
}

//
// Created by 潘鑫 on 2026/8/15.
//

#ifndef HELLO_MAC_RENDER_IMAGE_MANAGER_H
#define HELLO_MAC_RENDER_IMAGE_MANAGER_H
#include "image_and_view_paramter.h"
#include "vulkan_backend.h"
#include "vulkan_image.h"
#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"


class Render_image_manager {
    struct Using_of_Free {
        std::vector<VKR_image_ptr> is_using;
        std::vector<VKR_image_ptr> is_free;
    };

    absl::flat_hash_map<Image_and_view_parameters, Using_of_Free> map_;

public:
    void using_to_free();

    VKR_image_ptr get_one_position_image();

    VKR_image_ptr get_one_normal_image();

    VKR_image_ptr get_one_color_image();

    VKR_image_ptr get_one_entity_image();

    VKR_image_ptr get_one_shadow_image();

    VKR_image_ptr get_one_compute_write_image();

    void create();

    VKR_image_ptr get_one_depth_image();

    VKR_image_ptr get_one_depth_AO_image();

    VKR_image_ptr get_one_depth_SSAO_image();

    VKR_image_ptr find(const Image_and_view_parameters &parameters);

    void destroy();
};

#endif //HELLO_MAC_RENDER_IMAGE_MANAGER_H

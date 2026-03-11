//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_POOL_H
#define HOWTOVULKAN_DESCRIPTOR_POOL_H
#include "vulkan_backend.h"


void init_current_descriptor_pool();

VkDescriptorPool get_descriptor_pool();

void destroy_descriptorPool();

#endif //HOWTOVULKAN_DESCRIPTOR_POOL_H

// 整体销毁的顺序
// descriptor_pools_
// pipelines_
// pipeline_layouts_
// descriptor_sets_layout
// shader_modules_
// buffer_views_
// buffers_
// image_views_
// images_

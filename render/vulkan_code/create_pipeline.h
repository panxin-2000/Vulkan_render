//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_PIPELINE_H
#define HOWTOVULKAN_CREATE_PIPELINE_H
#include <vector>
#include <volk.h>

#include "shader_component.h"
#include "vulkan_backend.h"


/**
 * 没有具体的写明白这个函数
 * @param handle
 * @param shaderStages
 * @param descriptorSetLayout
 * @return
 */
VkPipeline CreateComputePipelines(VK_backend &handle,
                                  std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                                  VkDescriptorSetLayout &descriptorSetLayout);

VkPipeline create_graphics_pipeline(VK_backend &backend, vk_shader_data &data);

VkPipeline create_pipeline(VK_backend &backend, vk_shader_data &data);

VkPipeline find_pipeline(VK_backend &handle, shader_data &data);

void clean_all_pipeline(VK_backend &handle);

#endif //HOWTOVULKAN_CREATE_PIPELINE_H

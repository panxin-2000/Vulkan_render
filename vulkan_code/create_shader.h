//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_SHADER_H
#define HOWTOVULKAN_CREATE_SHADER_H

#include "vulkan_device_handle.h"
#include "shader_common.h"
#include "shader_component.h"

static std::vector<char> readFile(const std::string &filename);


VkShaderModule create_one_shader_module(const VK_handle &handle, const std::string &path,
                                        std::map<std::string, shader_and_share> &map);


VkShaderModule find_one_shader_module(const VK_handle &handle, const std::string &path,
                                      std::map<std::string, shader_and_share> &map);


std::vector<VkPipelineShaderStageCreateInfo> find_one_compute_shader_module(const VK_handle &handle,
                                                                            const std::string &compute_path);


std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_handle &handle,
                                                                         VKR_shader &paths);

void clean_all_shader_object(VK_handle &handle);

#endif //HOWTOVULKAN_CREATE_SHADER_H

//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_SHADER_H
#define HOWTOVULKAN_CREATE_SHADER_H

#include "vulkan_backend.h"
#include "shader_common.h"
#include "shader_resolve.h"

static std::vector<char> readFile(const std::string &filename);


VkShaderModule create_one_shader_module(const VK_backend &backend, const std::string &path);


VkShaderModule find_one_shader_module(const VK_backend &backend, const std::string &path,
                                      std::map<std::string, shader_and_share> &map);


std::vector<VkPipelineShaderStageCreateInfo> find_compute_shader_module(const VK_backend &backend,
                                                                        const VKR_shader_paths &paths);


std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_backend &backend,
                                                                         const VKR_shader_paths &paths);


#endif //HOWTOVULKAN_CREATE_SHADER_H

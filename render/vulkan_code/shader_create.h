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


std::vector<VkPipelineShaderStageCreateInfo> find_compute_shader_module(const VK_backend &backend,
                                                                        const VKR_shader_paths &paths);


std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_backend &backend,
                                                                         const VKR_shader_paths &paths);

std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_backend &backend,
                                                                         const VKR_shader_paths &paths,
                                                                         const Shader_data &shader_data_handle);


std::vector<VkPipelineShaderStageCreateInfo> find_compute_shader_module(const VK_backend &backend,
                                                                        const VKR_shader_paths &paths,
                                                                        const Shader_data &shader_data_handle);

#include <shaderc/shaderc.hpp>


std::vector<uint32_t> CompileGlslToSpv(const VKR_shader_paths &shader_paths,
                                       const std::string &filename,
                                       shaderc_shader_kind shader_kind);
#endif //HOWTOVULKAN_CREATE_SHADER_H

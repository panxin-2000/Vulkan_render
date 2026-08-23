//
// Created by 潘鑫 on 2026/2/8.
//

#ifndef HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H
#define HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H

#include "vulkan_backend.h"
#include <regex>
#include <filesystem>
#include <fstream>
#include <spirv_cross/spirv_glsl.hpp>

#include "shader_resolve.h"

VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings, const void *pNext = nullptr,
    const std::vector<VkDescriptorBindingFlags> &descriptor_binding_flags        = {});

VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
    const VkDescriptorType type,
    const VkShaderStageFlags stageFlags,
    const uint32_t binding,
    const uint32_t descriptorCount = 1);


VkShaderStageFlags get_stageFlags(const std::string &shaderStage);


std::pair<VkFormat, uint32_t> map_spirv_type_to_vk_format(const spirv_cross::SPIRType &type);


VkShaderStageFlags find_stageFlag(sets_map &sorted_sets_bindings, const std::string &binding_name);

void collect_and_sorted_vertex_input_resources(const spirv_cross::CompilerGLSL &compiler,
                                               spirv_cross::ShaderResources &resources,
                                               const std::string &shaderStage,
                                               std::vector<InputAttributeDescription> &vertexAttributes,
                                               std::vector<VkVertexInputBindingDescription> &vertexBindings);


VkFormat get_format_from_resource_name(const std::string &resource_name);


void collect_and_sorted_push_constant_resources(const spirv_cross::CompilerGLSL &compiler,
                                                spirv_cross::ShaderResources &resources,
                                                const std::string &shaderStage,
                                                Push_constant_map &push_constant_map);


void collect_and_sorted_fragment_output_resources(const spirv_cross::CompilerGLSL &compiler,
                                                  spirv_cross::ShaderResources &resources,
                                                  const std::string &shaderStage,
                                                  Fragment_output_map &ColorAttachment);

void collect_and_sorted_resources(const spirv_cross::CompilerGLSL &compiler,
                                  spirv_cross::ShaderResources &resources,
                                  const std::string &shaderStage,
                                  sets_map &bindless_bindings_set,
                                  sets_map &global_bindings_set,
                                  sets_map &sorted_sets_bindings);


void read_spv_file(std::vector<uint32_t> &spv_binary, const std::string &shaderStage,
                   sets_map &bindless_set,
                   sets_map &global_bindings_set_0,
                   sets_map &sorted_sets_bindings,
                   std::vector<InputAttributeDescription> &vertexAttributes,
                   std::vector<VkVertexInputBindingDescription> &vertexBindings,
                   Fragment_output_map &ColorAttachment,
                   Push_constant_map &push_constant_map);

void print_sorted_resources(const sets_map &sorted_sets_bindings);


void print_layout_binding_line(std::string filePath);


std::string get_shader_key(const VKR_shader_paths &paths);


//
sets_map organize_descriptor_set_and_binding_layouts(
    const VKR_shader_paths &paths, Shader_data &shader_data);


#endif //HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H

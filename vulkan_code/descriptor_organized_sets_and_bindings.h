//
// Created by 潘鑫 on 2026/2/8.
//

#ifndef HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H
#define HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H

#include "vulkan_device_handle.h"
#include <regex>
#include <filesystem>
#include <spirv_cross/spirv_glsl.hpp>
#define max_sets 8

static inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings, const void *pNext = nullptr,
    const std::vector<VkDescriptorBindingFlags> &descriptor_binding_flags        = {}) {
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};

    for (const auto &descriptor_binding_flag: descriptor_binding_flags) {
        if (descriptor_binding_flag & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT) {
            descriptorSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }
    }
    descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.pBindings    = bindings.data();
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetLayoutCreateInfo.pNext        = pNext; // 新增加的一行
    return descriptorSetLayoutCreateInfo;
}

static inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(
    const VkDescriptorType type,
    const VkShaderStageFlags stageFlags,
    const uint32_t binding,
    const uint32_t descriptorCount = 1) {
    VkDescriptorSetLayoutBinding setLayoutBinding{};
    setLayoutBinding.descriptorType  = type;
    setLayoutBinding.stageFlags      = stageFlags;
    setLayoutBinding.binding         = binding;
    setLayoutBinding.descriptorCount = descriptorCount;
    return setLayoutBinding;
}


struct binding_resource {
    VkDescriptorSetLayoutBinding LayoutBinding;
    std::string name;
    std::string resource_type; //  "uniform", "uniform sampler2D", "buffer", "uniform sampler" "uniform texture2D"
    std::string shaderStage;
    size_t need_allocate_size     = 0;
    VkDescriptorBindingFlags flag = 0;
};


inline VkShaderStageFlags get_stageFlags(const std::string &shaderStage) {
    if (shaderStage == "vertex") {
        return VK_SHADER_STAGE_VERTEX_BIT;
    } else if (shaderStage == "fragment") {
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (shaderStage == "geometry") {
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (shaderStage == "compute") {
        return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

static void collect_and_sorted_resources(const std::vector<uint32_t> &spirv_binary, std::string shaderStage,
                                         std::array<std::map<uint32_t, binding_resource>, max_sets> &sorted_bindings) {
    const spirv_cross::CompilerGLSL compiler(spirv_binary);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    // Use a map to automatically sort by Binding ID (the key)
    // 1. Collect Uniform Buffers
    for (const auto &res: resources.uniform_buffers) {
        uint32_t set                    = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding                = compiler.get_decoration(res.id, spv::DecorationBinding);
        const size_t need_allocate_size = compiler.get_declared_struct_size(compiler.get_type(res.type_id));
        VkDescriptorSetLayoutBinding tem{};
        tem.binding                   = binding;
        tem.descriptorCount           = 1;
        tem.stageFlags                = get_stageFlags(shaderStage);
        tem.descriptorType            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        sorted_bindings[set][binding] = {tem, res.name, "uniform", shaderStage, need_allocate_size};

        const auto &type = compiler.get_type(res.base_type_id);
        // 2. 遍历结构体内部的所有成员
        uint32_t member_count = type.member_types.size();
        for (uint32_t i = 0; i < member_count; i++) {
            // 获取成员名字（如 "projection"）
            std::string member_name = compiler.get_member_name(res.base_type_id, i);
            // 获取成员在内存中的偏移量（对你手动填充 Buffer 非常有用）
            uint32_t offset = compiler.type_struct_member_offset(type, i);
            // 获取成员的大小
            size_t size = compiler.get_declared_struct_member_size(type, i);
        }
    }
    // 2. Collect Storage Buffers
    for (const auto &res: resources.storage_buffers) {
        uint32_t set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding                   = binding;
        tem.descriptorCount           = 1;
        tem.stageFlags                = get_stageFlags(shaderStage);
        tem.descriptorType            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        sorted_bindings[set][binding] = {tem, res.name, "buffer", shaderStage, 0}; // SSBO size can be dynamic
    }
    // 3. Collect Sampled Images (Textures)
    for (const auto &res: resources.sampled_images) {
        uint32_t set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding                   = binding;
        tem.descriptorCount           = 1;
        tem.stageFlags                = get_stageFlags(shaderStage);
        tem.descriptorType            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        const auto &type              = compiler.get_type(res.type_id);
        VkDescriptorBindingFlags flag = 0;
        if (type.array.empty()) {
            // layout (binding = 1) uniform sampler2D sampler_position;
            tem.descriptorCount = 1;
        } else {
            // array[0] 存储的是最外层括号的长度
            uint32_t array_size = type.array[0];
            if (array_size == 0) {
                // layout (set = 0, binding = 0) uniform sampler2D samplerColorMap[];
                tem.descriptorCount = 100; // 这是一个上限，实际分配时， 暂时定义100，之后想办法添加一个宏吧
                flag                = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                                      VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
            } else {
                // layout (set = 0, binding = 0) uniform sampler2D samplerColorMap[5];
                tem.descriptorCount = array_size; // 暂时定义100，之后想办法添加一个宏吧
            }
        }
        sorted_bindings[set][binding] = {tem, res.name, "uniform sampler2D", shaderStage, 0, flag};
    }
    for (auto &res: resources.separate_samplers) {
        // layout(binding = 0) uniform sampler mySampler;
        uint32_t set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding                   = binding;
        tem.descriptorCount           = 1;
        tem.stageFlags                = get_stageFlags(shaderStage);
        tem.descriptorType            = VK_DESCRIPTOR_TYPE_SAMPLER;
        sorted_bindings[set][binding] = {tem, res.name, "uniform sampler", shaderStage, 0};
    }
    for (auto &res: resources.separate_images) {
        // layout(binding = 1) uniform texture2D myImage;
        auto &type = compiler.get_type(res.type_id);
        if (type.image.sampled == 1) {
            const uint32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            uint32_t binding   = compiler.get_decoration(res.id, spv::DecorationBinding);
            VkDescriptorSetLayoutBinding tem{};;
            tem.binding                   = binding;
            tem.descriptorCount           = 1;
            tem.stageFlags                = get_stageFlags(shaderStage);
            tem.descriptorType            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            sorted_bindings[set][binding] = {tem, res.name, "uniform texture2D", shaderStage, 0};
        }
    }
}


static void read_spv_file(const std::string &file_name, std::string shaderStage,
                          std::array<std::map<uint32_t, binding_resource>, max_sets> &sorted_bindings) {
    if (file_name.empty() == true) {
        return;
    }
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint32_t> spv_binary(size / sizeof(uint32_t));
    file.read(reinterpret_cast<char *>(spv_binary.data()), size);


    collect_and_sorted_resources(spv_binary, shaderStage, sorted_bindings);
}


static void print_sorted_resources(
    const std::array<std::map<uint32_t, binding_resource>, max_sets> &sorted_bindings_array) {
    // 4. Print results (Map iteration is always sorted by key)
    LOG_INFO(g_log(), "--- Resources Sorted by Binding ---");
    for (int i = 0; i < sorted_bindings_array.size(); i++) {
        auto sorted_bindings = sorted_bindings_array[i];
        auto set             = i;
        for (auto const &[binding, info]: sorted_bindings) {
            // layout (set = 0, binding = 0) uniform sampler2D samplerColorMap
            LOG_INFO(g_log(), "stage {}  : layout (set = {}, binding = {}) {} {}",
                     info.shaderStage, set, binding, info.resource_type, info.name);
        }
    }
}


void print_layout_binding_line(std::string filePath) {
    if (filePath.size() > 4 && filePath.substr(filePath.size() - 4) == ".spv") {
        filePath.erase(filePath.size() - 4); // 擦除最后4个字符
    }
    if (std::filesystem::exists(filePath)) {
        std::ifstream file(filePath);
        std::string line;
        // std::regex bindingRegex(R"(layout\s*\(.*binding\s*=\s*(\d+).*\))");
        // 匹配整行：必须同时包含 layout 和 binding
        std::regex bindingRegex(R"(^\s*layout\s*\(.*binding\s*=\s*(\d+).*\).*$)");

        while (std::getline(file, line)) {
            std::smatch match;
            if (std::regex_search(line, match, bindingRegex)) {
                std::string binding_id   = match[1].str(); // 第一个括号的内容
                std::string type_keyword = match[2].str(); // 第二个括号的内容
                std::string full_line    = match[0].str();
                LOG_INFO(g_log(), "{}", full_line);
            }
        }
    }
}


static std::array<std::map<uint32_t, binding_resource>, max_sets> organize_graphics_descriptor_set_and_binding_layouts(
    const std::string &vertex_path,
    const std::string &fragment_path,
    const std::string &geometry_path) {
    std::array<std::map<uint32_t, binding_resource>, max_sets> sorted_sets_and_bindings;
    if (!vertex_path.empty()) {
        LOG_INFO(g_log(), "--- vertex shader ---");

        read_spv_file(vertex_path, "vertex", sorted_sets_and_bindings);
        print_layout_binding_line(vertex_path);
    }
    if (!fragment_path.empty()) {
        LOG_INFO(g_log(), "--- fragment shader ---");

        read_spv_file(fragment_path, "fragment", sorted_sets_and_bindings);
        print_layout_binding_line(fragment_path);
    }
    if (!geometry_path.empty()) {
        LOG_INFO(g_log(), "--- geometry shader ---");

        read_spv_file(geometry_path, "geometry", sorted_sets_and_bindings);
        print_layout_binding_line(geometry_path);
    }
    print_sorted_resources(sorted_sets_and_bindings);
    return sorted_sets_and_bindings;
}

static std::array<std::map<uint32_t, binding_resource>, max_sets> organize_computer_descriptor_set_layouts(
    const std::string &computer_path) {
    //
    std::array<std::map<uint32_t, binding_resource>, max_sets> sorted_bindings;
    if (!computer_path.empty()) {
        LOG_INFO(g_log(), "--- computer shader ---");
        read_spv_file(computer_path, "computer", sorted_bindings);
        print_layout_binding_line(computer_path);
    }
    print_sorted_resources(sorted_bindings);
    return sorted_bindings;
}


#endif //HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H

//
// Created by 潘鑫 on 2026/2/8.
//

#ifndef HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H
#define HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H

#include "vulkan_device_handle.h"
#include <regex>
#include <filesystem>
#include <fstream>
#include <spirv_cross/spirv_glsl.hpp>

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


inline std::pair<VkFormat, uint32_t> map_spirv_type_to_vk_format(const spirv_cross::SPIRType &type) {
    using namespace spirv_cross;

    // Handle Floating Point (float, double)
    if (type.basetype == SPIRType::Float) {
        if (type.width == 32) {
            // 32-bit float
            switch (type.vecsize) {
                case 1: return {VK_FORMAT_R32_SFLOAT, 4 * 1};
                case 2: return {VK_FORMAT_R32G32_SFLOAT, 4 * 2};
                case 3: return {VK_FORMAT_R32G32B32_SFLOAT, 4 * 3};
                case 4: return {VK_FORMAT_R32G32B32A32_SFLOAT, 4 * 4};
            }
        } else if (type.width == 64) {
            // 64-bit double
            switch (type.vecsize) {
                case 1: return {VK_FORMAT_R64_SFLOAT, 8 * 1};
                case 2: return {VK_FORMAT_R64G64_SFLOAT, 8 * 2};
                case 3: return {VK_FORMAT_R64G64B64_SFLOAT, 8 * 3};
                case 4: return {VK_FORMAT_R64G64B64A64_SFLOAT, 8 * 4};
            }
        }
    }
    // Handle Unsigned Integer (uint)
    else if (type.basetype == SPIRType::UInt) {
        if (type.width == 32) {
            switch (type.vecsize) {
                case 1: return {VK_FORMAT_R32_UINT, 4 * 1};
                case 2: return {VK_FORMAT_R32G32_UINT, 4 * 2};
                case 3: return {VK_FORMAT_R32G32B32_UINT, 4 * 3};
                case 4: return {VK_FORMAT_R32G32B32A32_UINT, 4 * 4};
            }
        } else if (type.width == 8) {
            // Often used for packed colors
            switch (type.vecsize) {
                case 4: return {VK_FORMAT_R8G8B8A8_UINT, 1 * 4};
            }
        }
    }
    // Handle Signed Integer (int)
    else if (type.basetype == SPIRType::Int) {
        if (type.width == 32) {
            switch (type.vecsize) {
                case 1: return {VK_FORMAT_R32_SINT, 4 * 1};
                case 2: return {VK_FORMAT_R32G32_SINT, 4 * 2};
                case 3: return {VK_FORMAT_R32G32B32_SINT, 4 * 3};
                case 4: return {VK_FORMAT_R32G32B32A32_SINT, 4 * 4};
            }
        }
    }

    return {VK_FORMAT_UNDEFINED, 0};
}


static void collect_and_sorted_resources(const std::vector<uint32_t> &spirv_binary, const std::string &shaderStage,
                                         sets_map &global_bindings_set,
                                         sets_map &sorted_sets_bindings,
                                         std::vector<VkVertexInputAttributeDescription> &vertexAttributes,
                                         std::vector<VkVertexInputBindingDescription> &vertexBindings) {
    const spirv_cross::CompilerGLSL compiler(spirv_binary);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    // location 的解析
    if (shaderStage == "vertex") {
        std::map<uint32_t, std::pair<uint32_t, VkVertexInputAttributeDescription> > vertexAttributes_t;
        std::map<uint32_t, uint32_t> size_map;
        for (auto &resource: resources.stage_inputs) {
            // 1. Get the Name (e.g., "inPos")
            const std::string &name = resource.name;

            // 2. Get the Location (The '0', '1', '2' in your GLSL)
            uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            uint32_t binding  = compiler.get_decoration(resource.id, spv::DecorationBinding);

            // 3. Get the Type (e.g., vec3, vec2)
            auto &type                   = compiler.get_type(resource.type_id);
            auto [format, size]          = map_spirv_type_to_vk_format(type);
            vertexAttributes_t[location] = {size, {location, binding, format, 0}};
        }
        uint32_t total_offset = 0;
        for (auto [location,pair_data]: vertexAttributes_t) {
            pair_data.second.offset = total_offset;
            vertexAttributes.push_back(pair_data.second);
            total_offset = total_offset + pair_data.first;
        }
        if (total_offset != 0) {
            vertexBindings.push_back({
                                         .binding   = 0,
                                         .stride    = total_offset,
                                         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
                                     });
        }
    }

    // Use a map to automatically sort by Binding ID (the key)
    // 1. Collect Uniform Buffers
    for (const auto &res: resources.uniform_buffers) {
        uint32_t set                     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding                 = compiler.get_decoration(res.id, spv::DecorationBinding);
        const size_t need_allocate_size  = compiler.get_declared_struct_size(compiler.get_type(res.type_id));
        VkDescriptorSetLayoutBinding tem = {};
        tem.binding                      = binding;
        tem.descriptorCount              = 1;
        tem.stageFlags                   = get_stageFlags(shaderStage);
        tem.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        if (res.name.find("global") != std::string::npos) {
            global_bindings_set[set][binding] = {tem, res.name, "uniform buffer", shaderStage, need_allocate_size};
        } else {
            sorted_sets_bindings[set][binding] = {tem, res.name, "uniform buffer", shaderStage, need_allocate_size};
        }
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
        tem.binding         = binding;
        tem.descriptorCount = 1;
        tem.stageFlags      = get_stageFlags(shaderStage);
        tem.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        if (res.name.find("global") != std::string::npos) {
            global_bindings_set[set][binding] = {tem, res.name, "storage buffer", shaderStage, 0};
        } else {
            sorted_sets_bindings[set][binding] = {tem, res.name, "storage buffer", shaderStage, 0};
        }
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
        if (res.name.find("global") != std::string::npos) {
            global_bindings_set[set][binding] = {tem, res.name, "uniform sampler2D", shaderStage, 0, flag};
        } else {
            sorted_sets_bindings[set][binding] = {tem, res.name, "uniform sampler2D", shaderStage, 0, flag};
        }
    }
    for (auto &res: resources.separate_samplers) {
        // layout(binding = 0) uniform sampler mySampler;
        uint32_t set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding         = binding;
        tem.descriptorCount = 1;
        tem.stageFlags      = get_stageFlags(shaderStage);
        tem.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
        if (res.name.find("global") != std::string::npos) {
            global_bindings_set[set][binding] = {tem, res.name, "uniform sampler", shaderStage, 0};
        } else {
            sorted_sets_bindings[set][binding] = {tem, res.name, "uniform sampler", shaderStage, 0};
        }
    }
    for (auto &res: resources.separate_images) {
        // layout(binding = 1) uniform texture2D myImage;
        auto &type = compiler.get_type(res.type_id);
        if (type.image.sampled == 1) {
            const uint32_t set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            uint32_t binding   = compiler.get_decoration(res.id, spv::DecorationBinding);
            VkDescriptorSetLayoutBinding tem{};;
            tem.binding         = binding;
            tem.descriptorCount = 1;
            tem.stageFlags      = get_stageFlags(shaderStage);
            tem.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            if (res.name.find("global") != std::string::npos) {
                global_bindings_set[set][binding] = {tem, res.name, "uniform texture2D", shaderStage, 0};
            } else {
                sorted_sets_bindings[set][binding] = {tem, res.name, "uniform texture2D", shaderStage, 0};
            }
        }
    }
}


static void read_spv_file(const std::string &file_name, const std::string &shaderStage,
                          sets_map &global_bindings_set_0,
                          sets_map &sorted_sets_bindings,
                          std::vector<VkVertexInputAttributeDescription> &vertexAttributes,
                          std::vector<VkVertexInputBindingDescription> &vertexBindings) {
    if (file_name.empty() == true) {
        return;
    }
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint32_t> spv_binary(size / sizeof(uint32_t));
    file.read(reinterpret_cast<char *>(spv_binary.data()), size);


    collect_and_sorted_resources(spv_binary, shaderStage, global_bindings_set_0, sorted_sets_bindings, vertexAttributes,
                                 vertexBindings);
}


static void print_sorted_resources(const sets_map &sorted_sets_bindings) {
    LOG_INFO(g_log(), "--- Resources Sorted by Binding ---");
    for (auto const &[set_value, bindings_map]: sorted_sets_bindings) {
        auto sorted_bindings = bindings_map;
        auto set             = set_value;
        for (auto const &[binding_value, info]: sorted_bindings) {
            // layout (set = 0, binding = 0) uniform sampler2D samplerColorMap
            LOG_INFO(g_log(), "stage {}  : layout (set = {}, binding = {}) {} {}",
                     info.shaderStage, set, binding_value, info.resource_type, info.binding_name);
        }
    }
}


inline void print_layout_binding_line(std::string filePath) {
    std::string target = ".spv";
    size_t pos         = filePath.find(target);
    if (filePath.size() > 4 && pos != std::string::npos) {
        filePath.erase(pos, target.length());
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


inline std::string get_shader_key(const VKR_shader &paths) {
    const std::string &vertex_path   = paths.vertex_path_;
    const std::string &fragment_path = paths.fragment_path_;
    const std::string &geometry_path = paths.geometry_path_;
    const std::string &computer_path = paths.computer_path_;

    std::string temp_vertex_path   = std::filesystem::path(vertex_path).filename().string();
    std::string temp_fragment_path = std::filesystem::path(fragment_path).filename().string();
    std::string temp_geometry_path = std::filesystem::path(geometry_path).filename().string();
    std::string temp_computer_path = std::filesystem::path(computer_path).filename().string();

    std::string target = ".spv"; {
        size_t pos = temp_vertex_path.find(target);
        if (temp_vertex_path.size() > 4 && pos != std::string::npos) {
            temp_vertex_path.erase(pos, target.length());
        }
    } {
        size_t pos = temp_fragment_path.find(target);
        if (temp_fragment_path.size() > 4 && pos != std::string::npos) {
            temp_fragment_path.erase(pos, target.length());
        }
    } {
        size_t pos = temp_geometry_path.find(target);
        if (temp_geometry_path.size() > 4 && pos != std::string::npos) {
            temp_geometry_path.erase(pos, target.length());
        }
    } {
        size_t pos = temp_computer_path.find(target);
        if (temp_computer_path.size() > 4 && pos != std::string::npos) {
            temp_computer_path.erase(pos, target.length());
        }
    }
    return temp_vertex_path + temp_fragment_path + geometry_path + computer_path;
}


//
static sets_map organize_descriptor_set_and_binding_layouts(
    VKR_shader &paths) {
    const std::string &vertex_path   = paths.vertex_path_;
    const std::string &fragment_path = paths.fragment_path_;
    const std::string &geometry_path = paths.geometry_path_;
    const std::string &computer_path = paths.computer_path_;

    sets_map sorted_sets_bindings;
    sets_map &global_bindings_set = paths.shader_data_handle->global_bindings_set;
    std::vector<VkVertexInputBindingDescription> vertexBindings;
    std::vector<VkVertexInputAttributeDescription> vertexAttributes;
    if (!vertex_path.empty()) {
        LOG_INFO(g_log(), "--- vertex shader ---");
        read_spv_file(vertex_path, "vertex",
                      global_bindings_set,
                      sorted_sets_bindings,
                      vertexAttributes,
                      vertexBindings);
        print_layout_binding_line(vertex_path);
    }
    if (!fragment_path.empty()) {
        LOG_INFO(g_log(), "--- fragment shader ---");
        read_spv_file(fragment_path, "fragment",
                      global_bindings_set,
                      sorted_sets_bindings,
                      vertexAttributes,
                      vertexBindings);
        print_layout_binding_line(fragment_path);
    }
    if (!geometry_path.empty()) {
        LOG_INFO(g_log(), "--- geometry shader ---");
        read_spv_file(geometry_path, "geometry",
                      global_bindings_set,
                      sorted_sets_bindings,
                      vertexAttributes,
                      vertexBindings);
        print_layout_binding_line(geometry_path);
    }
    if (!computer_path.empty()) {
        LOG_INFO(g_log(), "--- computer shader ---");
        read_spv_file(computer_path, "computer",
                      global_bindings_set,
                      sorted_sets_bindings,
                      vertexAttributes,
                      vertexBindings);
        print_layout_binding_line(computer_path);
    }
    paths.shader_data_handle->vertexAttributes = vertexAttributes;
    paths.shader_data_handle->vertexBindings   = vertexBindings;
#ifndef NDEBUG
    print_sorted_resources(sorted_sets_bindings);
#endif
    return sorted_sets_bindings;
}


#endif //HELLO_MAC_DESCRIPTOR_SET_LAYOUT_H

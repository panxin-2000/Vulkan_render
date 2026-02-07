//
// Created by 潘鑫 on 2026/1/25.
//

#ifndef HOWTOVULKAN_DESCRIPTOR_H
#define HOWTOVULKAN_DESCRIPTOR_H
#include "vulkan_device_handle.h"
#include "descriptor_pool.h"
#include <spirv_cross/spirv_glsl.hpp>


static inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayoutBinding> &bindings, const void *pNext = nullptr) {
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
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


static inline VkDescriptorSetLayout create_descriptor_set_layout(const VKDevice &handle,
                                                                 std::vector<VkDescriptorSetLayoutBinding>
                                                                 setLayoutBindings) {
    VkDescriptorSetLayout descriptorSetLayout;

    const VkDescriptorSetLayoutCreateInfo descriptorLayout = descriptorSetLayoutCreateInfo(setLayoutBindings);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
                        descriptorSetLayout));
    return descriptorSetLayout;
}


static inline VkDescriptorSetLayoutBindingFlagsCreateInfo DescriptorSetLayoutBindingFlagsCreateInfo(
    const std::vector<VkDescriptorBindingFlags> &descVariableFlags) {
    const VkDescriptorSetLayoutBindingFlagsCreateInfo descBindingFlags{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount  = static_cast<uint32_t>(descVariableFlags.size()),
        .pBindingFlags = descVariableFlags.data(),
    };
    return descBindingFlags;
}

VkDescriptorSetLayout Create_texture_binding_lessLayout(const VKDevice &handle, uint32_t size) {
    // Descriptor (indexing)
    VkDescriptorSetLayout descriptorSetLayoutTex                      = VK_NULL_HANDLE;
    const std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   size),
    };
    const std::vector<VkDescriptorBindingFlags> descVariableFlags{
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
    };
    // descVariableFlags 要么没有，要么需要和 setLayoutBindings 一致
    const auto descBindingFlags = DescriptorSetLayoutBindingFlagsCreateInfo(descVariableFlags);
    const auto descriptorLayout = descriptorSetLayoutCreateInfo(setLayoutBindings, (void *) &descBindingFlags);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(handle.get_device(), &descriptorLayout, nullptr, &
                        descriptorSetLayoutTex));
    return descriptorSetLayoutTex;
}


VkPipelineLayout CreatePipelineLayout(const VKDevice &handle, VkDescriptorSetLayout descriptorSetLayoutTex) {
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPushConstantRange pushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .size = sizeof(VkDeviceAddress)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutCI{
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &descriptorSetLayoutTex,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &pushConstantRange
    };
    VK_CHECK_RESULT(vkCreatePipelineLayout(handle.get_device(), &pipelineLayoutCI, nullptr, &pipelineLayout));
    return pipelineLayout;
}


void update_descriptor_sets(const VKDevice &handle, std::vector<VkDescriptorImageInfo> textureDescriptors,
                            std::vector<VkDescriptorSet> descriptor_set_texture) {
    std::vector<VkWriteDescriptorSet> writeDescSet;
    for (uint32_t i = 0; i < descriptor_set_texture.size(); i++) {
        VkWriteDescriptorSet temp{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = descriptor_set_texture[i],
            .dstBinding      = 0,
            .descriptorCount = static_cast<uint32_t>(textureDescriptors.size()),
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = textureDescriptors.data()
        };
        writeDescSet.push_back(temp);
    }

    vkUpdateDescriptorSets(handle.get_device(),
                           writeDescSet.size(),
                           writeDescSet.data(), 0, nullptr);
}


// std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
//     descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
// };

struct ResourceInfo {
    VkDescriptorSetLayoutBinding LayoutBinding;
    std::string name;
    std::string type; // e.g., "UBO", "SSBO"
    std::string shaderStage;
    size_t need_allocate_size = 0;
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
                                         std::map<uint32_t, ResourceInfo> &sorted_bindings) {
    const spirv_cross::CompilerGLSL compiler(spirv_binary);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    // Use a map to automatically sort by Binding ID (the key)
    // 1. Collect Uniform Buffers
    for (const auto &res: resources.uniform_buffers) {
        uint32_t binding  = compiler.get_decoration(res.id, spv::DecorationBinding);
        const size_t size = compiler.get_declared_struct_size(compiler.get_type(res.type_id));
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding              = binding;
        tem.descriptorCount      = 1;
        tem.stageFlags           = get_stageFlags(shaderStage);
        tem.stageFlags           = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        sorted_bindings[binding] = {tem, res.name, "Uniform Buffer", shaderStage, size};

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
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding              = binding;
        tem.descriptorCount      = 1;
        tem.stageFlags           = get_stageFlags(shaderStage);
        tem.stageFlags           = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        sorted_bindings[binding] = {tem, res.name, "Storage Buffer", shaderStage, 0}; // SSBO size can be dynamic
    }
    // 3. Collect Sampled Images (Textures)
    for (const auto &res: resources.sampled_images) {
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding         = binding;
        tem.descriptorCount = 1;
        tem.stageFlags      = get_stageFlags(shaderStage);
        tem.stageFlags      = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        const auto &type    = compiler.get_type(res.type_id);
        if (type.array.empty()) {
            // layout (binding = 1) uniform sampler2D sampler_position;
            tem.descriptorCount = 1;
        } else {
            // array[0] 存储的是最外层括号的长度
            uint32_t array_size = type.array[0];
            if (array_size == 0) {
                // layout (set = 0, binding = 0) uniform sampler2D samplerColorMap[];
                tem.descriptorCount = 100; // 暂时定义100，之后想办法添加一个宏吧
            } else {
                // layout (set = 0, binding = 0) uniform sampler2D samplerColorMap[5];
                tem.descriptorCount = array_size; // 暂时定义100，之后想办法添加一个宏吧
            }
        }
        sorted_bindings[binding] = {tem, res.name, "Texture/Sampler", shaderStage, 0};
    }
    for (auto &res: resources.separate_samplers) {
        // layout(binding = 0) uniform sampler mySampler;
        uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        VkDescriptorSetLayoutBinding tem{};;
        tem.binding              = binding;
        tem.descriptorCount      = 1;
        tem.stageFlags           = get_stageFlags(shaderStage);
        tem.stageFlags           = VK_DESCRIPTOR_TYPE_SAMPLER;
        sorted_bindings[binding] = {tem, res.name, "Sampler", shaderStage, 0};
    }
    for (auto &res: resources.separate_images) {
        // layout(binding = 1) uniform texture2D myImage;
        auto &type = compiler.get_type(res.type_id);
        if (type.image.sampled == 1) {
            uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            VkDescriptorSetLayoutBinding tem{};;
            tem.binding              = binding;
            tem.descriptorCount      = 1;
            tem.stageFlags           = get_stageFlags(shaderStage);
            tem.stageFlags           = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            sorted_bindings[binding] = {tem, res.name, "Texture", shaderStage, 0};
        }
    }
}

static void read_spv_file(const std::string &file_name, std::string shaderStage,
                          std::map<uint32_t, ResourceInfo> &sorted_bindings) {
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

static void print_sorted_resources(const std::map<uint32_t, ResourceInfo> &sorted_bindings) {
    // 4. Print results (Map iteration is always sorted by key)
    std::cout << "--- Resources Sorted by Binding ---" << std::endl;
    for (auto const &[binding, info]: sorted_bindings) {
        std::cout << "stage " << info.shaderStage << " "
                << "Binding [" << binding << "]: "
                << info.name << " (" << info.type << ")";
        if (info.need_allocate_size > 0) std::cout << " | Size: " << info.need_allocate_size << " bytes";
        std::cout << std::endl;
    }
}


std::vector<VkDescriptorSet> AllocateDescriptorSets(VKDevice &handle, uint32_t size,
                                                    VkDescriptorSetLayout descriptorSetLayout) {
    std::vector<VkDescriptorSet> descriptor_set_texture;
    std::vector<uint32_t> variableDescCount{size, size};
    std::vector<VkDescriptorSetLayout> layouts{descriptorSetLayout, descriptorSetLayout};

    // Vulkan 协议强制规定：只有索引号（Binding Number）最大的那一个绑定可以是可变的
    // 位置限制： 只有描述符集布局中 Binding 编号最大 的那个绑定才能设置为可变长度。
    // 上限约束： 你在 pDescriptorCounts 中指定的数值，不能超过你在 VkDescriptorSetLayoutBinding 中定义的 descriptorCount（即最大上限）。
    // 特性开启： 需要在物理设备特性中开启 descriptorIndexing 的相关支持，具体可参考 Vulkan 硬件数据库 检查你的显卡是否支持 runtimeDescriptorArray
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
        .descriptorSetCount = static_cast<uint32_t>(variableDescCount.size()),
        .pDescriptorCounts  = variableDescCount.data(),
    };

    descriptor_set_texture.resize(layouts.size());

    VkDescriptorSetAllocateInfo texDescSetAlloc{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = &variableDescCountAI,
        .descriptorPool     = handle.get_descriptor_pool(),
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()), // // 打算分配的集合数量
        .pSetLayouts        = layouts.data(),                        // 指向布局数组的指针,长度必须等于 descriptorSetCount
    };
    VK_CHECK_RESULT(vkAllocateDescriptorSets(handle.get_device(), &texDescSetAlloc, descriptor_set_texture.data()));
    return descriptor_set_texture;
}

static void create_descriptor_set_layouts(const VKDevice &handle,
                                          const std::string &vertex_path,
                                          const std::string &fragment_path,
                                          const std::string &geometry_path) {
    std::map<uint32_t, ResourceInfo> sorted_bindings;
    if (!vertex_path.empty()) {
        read_spv_file(vertex_path, "vertex", sorted_bindings);
    }
    if (!fragment_path.empty()) {
        read_spv_file(fragment_path, "fragment", sorted_bindings);
    }
    if (!geometry_path.empty()) {
        read_spv_file(geometry_path, "geometry", sorted_bindings);
    }
    print_sorted_resources(sorted_bindings);
    // 现在有了排列好的结果，那么应该就可以开始申请了。
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
    for (const auto &[fst, snd]: sorted_bindings) {
        setLayoutBindings.push_back(snd.LayoutBinding);
    }
    auto SetLayout = create_descriptor_set_layout(handle, setLayoutBindings);
}

#endif //HOWTOVULKAN_DESCRIPTOR_H

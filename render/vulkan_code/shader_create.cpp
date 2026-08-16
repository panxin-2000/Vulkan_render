//
// Created by 潘鑫 on 2026/3/3.
//

#include "shader_create.h"
#include <fstream>


static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


VkShaderModule create_one_shader_module(const VK_backend &backend, const std::string &path) {
    if (!path.empty()) {
        const auto shader_code = readFile(path);
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shader_code.size();
        createInfo.pCode    = reinterpret_cast<const uint32_t *>(shader_code.data());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        auto vk_result              = vkCreateShaderModule(backend.get_device(), &createInfo, nullptr, &shaderModule);
        if (vk_result != VK_SUCCESS) {
            return VK_NULL_HANDLE;
            LOG_ERROR(g_log(), "vkCreateShaderModule error {}!", path);
        }
        return shaderModule;
    }
    return VK_NULL_HANDLE;
}


void create_Shader_Module(const VK_backend &backend, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                          const std::string &shader_path, const VkShaderStageFlagBits stage) {
    if (shader_path.empty())
        return;
    VkShaderModule ShaderModule = create_one_shader_module(backend, shader_path);
    if (ShaderModule != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.stage  = stage;
        ShaderStageInfo.module = ShaderModule;
        ShaderStageInfo.pName  = "main";
        shaderStages.push_back(ShaderStageInfo);
    }
}


std::vector<VkPipelineShaderStageCreateInfo> find_compute_shader_module(const VK_backend &backend,
                                                                        const VKR_shader_paths &paths) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    create_Shader_Module(backend, shaderStages, paths.compute_path_, VK_SHADER_STAGE_COMPUTE_BIT);
    return shaderStages;
}


std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_backend &backend,
                                                                         const VKR_shader_paths &paths) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    create_Shader_Module(backend, shaderStages, paths.vertex_path_, VK_SHADER_STAGE_VERTEX_BIT);
    create_Shader_Module(backend, shaderStages, paths.fragment_path_, VK_SHADER_STAGE_FRAGMENT_BIT);
    create_Shader_Module(backend, shaderStages, paths.geometry_path_, VK_SHADER_STAGE_GEOMETRY_BIT);
    return shaderStages;
}

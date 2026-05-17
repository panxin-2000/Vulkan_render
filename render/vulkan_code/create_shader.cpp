//
// Created by 潘鑫 on 2026/3/3.
//

#include "create_shader.h"
#include <fstream>

std::map<std::string, shader_and_share> shader_maps_;

auto &get_shader_map() {
    return shader_maps_;
}

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


VkShaderModule create_one_shader_module(const VK_backend &backend, const std::string &path,
                                        std::map<std::string, shader_and_share> &map) {
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
        } else {
            auto it = map.find(path);
            if (it != map.end()) {
                it->second.shared_number++;
            } else {
                map.insert({path, {shaderModule, 1}});
            }
        }
        return shaderModule;
    }
    return VK_NULL_HANDLE;
}

VkShaderModule find_one_shader_module(const VK_backend &backend, const std::string &path,
                                      std::map<std::string, shader_and_share> &map) {
    auto it = map.find(path);
    if (it != map.end()) {
        return it->second.shader;
    }
    return create_one_shader_module(backend, path, map);
}


void create_Shader_Module(const VK_backend &backend, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                          const std::string &shader_path, const VkShaderStageFlagBits stage) {
    if (shader_path.empty())
        return;
    VkShaderModule ShaderModule = find_one_shader_module(backend, shader_path, get_shader_map());
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
    create_Shader_Module(backend, shaderStages, paths.computer_path_, VK_SHADER_STAGE_COMPUTE_BIT);
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

void clean_all_shader_object(VK_backend &handle) {
    // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    for (const auto &[key, value]: get_shader_map()) {
        vkDestroyShaderModule(handle.get_device(), value.shader, nullptr);
    }
    get_shader_map().clear();
}

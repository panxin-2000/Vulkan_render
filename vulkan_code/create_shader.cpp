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


VkShaderModule create_one_shader_module(const VK_handle &handle, const std::string &path,
                                        std::map<std::string, shader_and_share> &map) {
    if (!path.empty()) {
        const auto shader_code = readFile(path);
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shader_code.size();
        createInfo.pCode    = reinterpret_cast<const uint32_t *>(shader_code.data());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        auto vk_result              = vkCreateShaderModule(handle.get_device(), &createInfo, nullptr, &shaderModule);
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

VkShaderModule find_one_shader_module(const VK_handle &handle, const std::string &path,
                                      std::map<std::string, shader_and_share> &map) {
    auto it = map.find(path);
    if (it != map.end()) {
        return it->second.shader;
    }
    return create_one_shader_module(handle, path, map);
}


std::vector<VkPipelineShaderStageCreateInfo> find_one_compute_shader_module(const VK_handle &handle,
                                                                            const std::string &compute_path) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkShaderModule computeShaderModule = find_one_shader_module(handle, compute_path, get_shader_map());
    if (computeShaderModule != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
        ShaderStageInfo.module = computeShaderModule;
        ShaderStageInfo.pName  = "main";
        shaderStages.push_back(ShaderStageInfo);
    }
    return shaderStages;
}


std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_handle &handle,
                                                                         VKR_shader_paths &paths) {
    const std::string &vertex_path   = paths.vertex_path_;
    const std::string &fragment_path = paths.fragment_path_;
    const std::string &geometry_path = paths.geometry_path_;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    VkShaderModule vertShaderModule       = find_one_shader_module(handle, vertex_path, get_shader_map());
    VkShaderModule fragShaderModule       = find_one_shader_module(handle, fragment_path, get_shader_map());
    VkShaderModule geometry_shader_module = find_one_shader_module(handle, geometry_path, get_shader_map());

    if (vertShaderModule != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        ShaderStageInfo.module = vertShaderModule;
        ShaderStageInfo.pName  = "main";
        shaderStages.push_back(ShaderStageInfo);
    }
    if (fragShaderModule != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        ShaderStageInfo.module = fragShaderModule;
        ShaderStageInfo.pName  = "main"; //运行我们把多个着色器程序放到一个文件中
        shaderStages.push_back(ShaderStageInfo);
    }
    if (geometry_shader_module != VK_NULL_HANDLE) {
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
        ShaderStageInfo.module = geometry_shader_module;
        ShaderStageInfo.pName  = "main";
        shaderStages.push_back(ShaderStageInfo);
    }
    return shaderStages;
}

void clean_all_shader_object(VK_handle &handle) {
    // 正式项目中，确保 vkDeviceWaitIdle 后按顺序销毁资源是专业开发者的标准做法
    for (const auto &[key, value]: get_shader_map()) {
        vkDestroyShaderModule(handle.get_device(), value.shader, nullptr);
    }
    get_shader_map().clear();
}

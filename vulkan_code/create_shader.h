//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_SHADER_H
#define HOWTOVULKAN_CREATE_SHADER_H
#include <iostream>
#include <vector>
#include <volk.h>
#include <fstream>

#include "logic_render_data.h"
#include "vulkan_device_handle.h"

// #include "/usr/local/lib/slang/include/slang.h"
// #include "/usr/local/lib/slang/include/slang-com-ptr.h"
// Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;

// inline VkShaderModule create_shader_module(const VKDevice &handle, std::string path) {
//     std::vector<VkPipelineShaderStageCreateInfo> stages;
//
//     // Initialize Slang shader compiler
//     slang::createGlobalSession(slangGlobalSession.writeRef());
//     std::vector<slang::TargetDesc> slangTargets{
//         {
//             .format{SLANG_SPIRV},
//             .profile{slangGlobalSession->findProfile("spirv_1_4")}
//         }
//     };
//     std::vector<slang::CompilerOptionEntry> slangOptions{
//         {
//             slang::CompilerOptionName::EmitSpirvDirectly,
//             {slang::CompilerOptionValueKind::Int, 1}
//         }
//     };
//     slang::SessionDesc slangSessionDesc{
//         .targets{slangTargets.data()}, .targetCount{SlangInt(slangTargets.size())},
//         .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR, .compilerOptionEntries{slangOptions.data()},
//         .compilerOptionEntryCount{uint32_t(slangOptions.size())}
//     };
//     // Load shader
//     Slang::ComPtr<slang::ISession> slangSession;
//     slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());
//     Slang::ComPtr<slang::IModule> slangModule{
//         slangSession->loadModuleFromSource("triangle", path.c_str(), nullptr, nullptr)
//     };
//     Slang::ComPtr<ISlangBlob> spirv;
//     slangModule->getTargetCode(0, spirv.writeRef());
//
//     VkShaderModuleCreateInfo shaderModuleCI{
//         .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
//         .codeSize = spirv->getBufferSize(),
//         .pCode    = (uint32_t *) spirv->getBufferPointer()
//     };
//     VkShaderModule shaderModule = VK_NULL_HANDLE;
//     VK_CHECK_RESULT_NOT_EXIT(vkCreateShaderModule(handle.get_device(), &shaderModuleCI, nullptr, &shaderModule));
//     return shaderModule;
// }

// std::vector<VkPipelineShaderStageCreateInfo> createShaderStages(VkShaderModule shaderModule) {
//     std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
//         {
//             .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//             .stage  = VK_SHADER_STAGE_VERTEX_BIT,
//             .module = shaderModule,
//             .pName  = "main"
//         },
//         {
//             .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//             .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
//             .module = shaderModule,
//             .pName  = "main"
//         }
//     };
//
//     return shaderStages;
// }

// VkShaderModule createShaderModule(const VKDevice &handle, const std::vector<char> &code) {
//     VkShaderModuleCreateInfo createInfo{};
//     createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//     createInfo.codeSize = code.size();
//     createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());
//
//     VkShaderModule shaderModule;
//     if (vkCreateShaderModule(handle.get_device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create shader module!");
//     }
//     VKDevice::get().get_shader_map();
//
//
//     return shaderModule;
// }

// 之后再优化函数
// std::optional<VkShaderModule> create_shader_module(const VKDevice &handle, const std::vector<char> &code) {
//     VkShaderModuleCreateInfo createInfo{};
//     createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//     createInfo.codeSize = code.size();
//     createInfo.pCode    = reinterpret_cast<const uint32_t *>(code.data());
//     VkShaderModule shaderModule;
//     if (vkCreateShaderModule(handle.get_device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
//         return {};
//     }
//     return shaderModule;
// }

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


inline VkShaderModule create_one_shader_module(const VKDevice &handle, const std::string &path,
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

inline VkShaderModule find_one_shader_module(const VKDevice &handle, const std::string &path,
                                             std::map<std::string, shader_and_share> &map) {
    auto it = map.find(path);
    if (it != map.end()) {
        return it->second.shader;
    }
    return create_one_shader_module(handle, path, map);
}


inline std::vector<VkPipelineShaderStageCreateInfo> find_one_compute_shader_module(const VKDevice &handle,
    const std::string &compute_path) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkShaderModule computeShaderModule = find_one_shader_module(handle, compute_path,
                                                                VKDevice::get().get_shader_map());
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


inline std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VKDevice &handle,
    const std::string &vertex_path,
    const std::string &fragment_path,
    const std::string &geometry_path) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    VkShaderModule vertShaderModule = find_one_shader_module(handle, vertex_path,
                                                             VKDevice::get().get_shader_map());
    VkShaderModule fragShaderModule = find_one_shader_module(handle, fragment_path,
                                                             VKDevice::get().get_shader_map());
    VkShaderModule geometry_shader_module = find_one_shader_module(handle, geometry_path,
                                                                   VKDevice::get().get_shader_map());

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


#endif //HOWTOVULKAN_CREATE_SHADER_H

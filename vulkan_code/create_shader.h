//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_SHADER_H
#define HOWTOVULKAN_CREATE_SHADER_H
#include <vector>
#include <volk.h>

#include "/usr/local/lib/slang/include/slang.h"
#include "/usr/local/lib/slang/include/slang-com-ptr.h"
#include "vulkan_device_handle.h"

Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;


inline VkShaderModule create_shader_module(const VKDevice &handle, std::string path) {
    std::vector<VkPipelineShaderStageCreateInfo> stages;

    // Initialize Slang shader compiler
    slang::createGlobalSession(slangGlobalSession.writeRef());
    std::vector<slang::TargetDesc> slangTargets{
        {
            .format{SLANG_SPIRV},
            .profile{slangGlobalSession->findProfile("spirv_1_4")}
        }
    };
    std::vector<slang::CompilerOptionEntry> slangOptions{
        {
            slang::CompilerOptionName::EmitSpirvDirectly,
            {slang::CompilerOptionValueKind::Int, 1}
        }
    };
    slang::SessionDesc slangSessionDesc{
        .targets{slangTargets.data()}, .targetCount{SlangInt(slangTargets.size())},
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR, .compilerOptionEntries{slangOptions.data()},
        .compilerOptionEntryCount{uint32_t(slangOptions.size())}
    };
    // Load shader
    Slang::ComPtr<slang::ISession> slangSession;
    slangGlobalSession->createSession(slangSessionDesc, slangSession.writeRef());
    Slang::ComPtr<slang::IModule> slangModule{
        slangSession->loadModuleFromSource("triangle", path.c_str(), nullptr, nullptr)
    };
    Slang::ComPtr<ISlangBlob> spirv;
    slangModule->getTargetCode(0, spirv.writeRef());

    VkShaderModuleCreateInfo shaderModuleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv->getBufferSize(),
        .pCode = (uint32_t *) spirv->getBufferPointer()
    };
    VkShaderModule shaderModule{};
    VK_CHECK_RESULT(vkCreateShaderModule(handle.get_device(), &shaderModuleCI, nullptr, &shaderModule));
    return shaderModule;
}

std::vector<VkPipelineShaderStageCreateInfo> createShaderStages(VkShaderModule shaderModule) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shaderModule,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shaderModule,
            .pName = "main"
        }
    };

    return shaderStages;
}

VkShaderModule createShaderModule(const VKDevice &handle, const std::vector<char> &code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(handle.get_device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
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

inline VkShaderModule create_shader_module(const VKDevice &handle,
                                           const std::string &vertex_path,
                                           const std::string &fragment_path,
                                           const std::string &geometry_path) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    if (!vertex_path.empty() && !fragment_path.empty()) {
        auto vertShaderCode = readFile(vertex_path);
        auto fragShaderCode = readFile(fragment_path);
        // 原本问题在这里，没有办法正确的读取文件
        // 还需要正确配置路径

        VkShaderModule vertShaderModule = createShaderModule(handle, vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(handle, fragShaderCode);


        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main"; //运行我们把多个着色器程序放到一个文件中
        shaderStages.push_back(vertShaderStageInfo);
        shaderStages.push_back(fragShaderStageInfo);
    }
    if (!geometry_path.empty()) {
        auto geometryShaderCode = readFile(geometry_path);
        VkShaderModule temp = createShaderModule(handle, geometryShaderCode);
        VkPipelineShaderStageCreateInfo ShaderStageInfo{};
        ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        ShaderStageInfo.module = temp;
        ShaderStageInfo.pName = "main";
        shaderStages.push_back(ShaderStageInfo);
    }
}


#endif //HOWTOVULKAN_CREATE_SHADER_H

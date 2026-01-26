//
// Created by 潘鑫 on 2026/1/26.
//

#ifndef HOWTOVULKAN_CREATE_SHADER_H
#define HOWTOVULKAN_CREATE_SHADER_H
#include <vector>
#include <volk.h>

#include "/usr/local/lib/slang/include/slang.h"
#include "/usr/local/lib/slang/include/slang-com-ptr.h"

Slang::ComPtr<slang::IGlobalSession> slangGlobalSession;


VkShaderModule createshaderModule(VKDevice &handle) {
    std::vector<VkPipelineShaderStageCreateInfo> stages;

    // Initialize Slang shader compiler
    slang::createGlobalSession(slangGlobalSession.writeRef());
    auto slangTargets{
        std::to_array<slang::TargetDesc>({
            {.format{SLANG_SPIRV}, .profile{slangGlobalSession->findProfile("spirv_1_4")}}
        })
    };
    auto slangOptions{
        std::to_array<slang::CompilerOptionEntry>({
            {slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1}}
        })
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
        slangSession->loadModuleFromSource("triangle", "assets/shader.slang", nullptr, nullptr)
    };
    Slang::ComPtr<ISlangBlob> spirv;
    slangModule->getTargetCode(0, spirv.writeRef());

    VkShaderModuleCreateInfo shaderModuleCI{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv->getBufferSize(),
        .pCode = (uint32_t *) spirv->getBufferPointer()
    };
    VkShaderModule shaderModule{};
    chk(vkCreateShaderModule(handle.get_device(), &shaderModuleCI, nullptr, &shaderModule));
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


#endif //HOWTOVULKAN_CREATE_SHADER_H

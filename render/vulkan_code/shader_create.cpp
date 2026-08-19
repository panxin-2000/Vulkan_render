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

#include <shaderc/shaderc.hpp>

class CustomShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    // 构造函数传入你的 Shader 根目录（例如 "assets/shaders/"）
    explicit CustomShaderIncluder(std::string base_dir) : m_base_dir(std::move(base_dir)) {
        if (!m_base_dir.empty() && m_base_dir.back() != '/') {
            m_base_dir += '/';
        }
    }

    // 🚀 核心回调函数：当 Shaderc 在源码里遇到 #include 时，会自动触发这个函数
    shaderc_include_result *GetInclude(
        const char *requested_source,  // 被引用的头文件名（如 "global_shader_common.glsl"）
        shaderc_include_type type,     // 引用类型（"..." 或 <..._>）
        const char *requesting_source, // 谁引用了它（主 Shader 的标签名）
        size_t include_depth           // 嵌套深度
    ) override {
        // 1. 拼接物理磁盘路径
        std::string full_path = m_base_dir + requested_source;

        // 2. 读取文件内容到内存中
        std::ifstream file(full_path);
        auto *data = new shaderc_include_result();

        if (!file.is_open()) {
            // 如果文件打不开，必须返回一个包含错误信息的结构体，否则程序会崩溃
            std::string error_msg = "Cannot open include file: " + full_path;
            char *error_ptr       = new char[error_msg.size() + 1];
            std::copy(error_msg.begin(), error_msg.end(), error_ptr);
            error_ptr[error_msg.size()] = '\0';

            data->source_name        = requested_source;
            data->source_name_length = strlen(requested_source);
            data->content            = error_ptr; // 把错误信息当作内容返回，Shaderc 会在编译报错里打印出来
            data->content_length     = error_msg.size();
            data->user_data          = error_ptr; // 用于在 Release 阶段释放内存
            return data;
        }

        std::stringstream ss;
        ss << file.rdbuf();
        std::string content = ss.str();

        // 3. 动态分配内存给内容字符串，因为这些数据在编译完成前不能被析构
        char *content_ptr = new char[content.size() + 1];
        std::copy(content.begin(), content.end(), content_ptr);
        content_ptr[content.size()] = '\0';

        char *path_ptr = new char[full_path.size() + 1];
        std::copy(full_path.begin(), full_path.end(), path_ptr);
        path_ptr[full_path.size()] = '\0';

        // 4. 填充结构体
        data->source_name        = path_ptr;
        data->source_name_length = full_path.size();
        data->content            = content_ptr;
        data->content_length     = content.size();
        data->user_data          = data; // 把当前整个结构体指针存入，方便在 Release 时释放

        return data;
    }

    // 🚀 释放释放函数：当当前这个 #include 编译结束后，Shaderc 会自动调用这个函数来让我们清理内存
    void ReleaseInclude(shaderc_include_result *data) override {
        if (data) {
            // 如果在 GetInclude 里分配了错误信息或资源，这里一定要删掉，防止多线程内存泄露
            if (data->user_data == data) {
                delete[] data->source_name;
                delete[] data->content;
                delete data;
            } else {
                // 处理错误信息的释放
                delete[] static_cast<char *>(data->user_data);
                delete data;
            }
        }
    }

private:
    std::string m_base_dir;
};


std::vector<uint32_t> CompileGlslToSpv(const std::string &filename, shaderc_shader_kind shader_kind) {
    // 2. 如果没找到，读取源码文件
    if (filename.empty())
        return {};
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source_code = buffer.str();
    if (source_code.empty()) {
        return {};
    }
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    // options.SetOptimizationLevel(shaderc_optimization_level_performance); // 开启性能优化

    std::filesystem::path filePath = filename;
    std::filesystem::path baseDir  = filePath.parent_path();

    options.SetIncluder(std::make_unique<CustomShaderIncluder>(baseDir));
    options.SetGenerateDebugInfo();

    // 🚀 根据不同的 Pass 运行时动态注入宏
    // if (pass == RenderPassType::Depth) {
    //     options.AddMacroDefinition("PASS_DEPTH", "1");
    // } else if (pass == RenderPassType::Picking) {
    //     options.AddMacroDefinition("PASS_PICKING", "1");
    // } else {
    //     options.AddMacroDefinition("PASS_COLOR", "1");
    // }

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
                                                                     source_code, shader_kind,
                                                                     filename.c_str(), options
                                                                    );
    auto status = result.GetCompilationStatus();
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << "编译发生错误: " << result.GetErrorMessage() << std::endl;
        return {};
    }
    std::vector<uint32_t> spirv_code(result.cbegin(), result.cend());
    return spirv_code;
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


VkShaderModule create_one_shader_module(const VK_backend &backend, const std::string &path,
                                        const std::vector<uint32_t> &spv_data) {
    if (!path.empty() && !spv_data.empty()) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spv_data.size() * sizeof(uint32_t);
        createInfo.pCode    = spv_data.data();

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

void create_Shader_Module(const VK_backend &backend, std::vector<VkPipelineShaderStageCreateInfo> &shaderStages,
                          const std::string &shader_path, const std::vector<uint32_t> &spv_data,
                          const VkShaderStageFlagBits stage) {
    if (shader_path.empty() && spv_data.empty())
        return;
    VkShaderModule ShaderModule = create_one_shader_module(backend, shader_path, spv_data);
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

std::vector<VkPipelineShaderStageCreateInfo> find_compute_shader_module(const VK_backend &backend,
                                                                        const VKR_shader_paths &paths,
                                                                        const Shader_data &shader_data_handle) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    create_Shader_Module(backend, shaderStages, paths.compute_path_, shader_data_handle->spv_data_comp,
                         VK_SHADER_STAGE_COMPUTE_BIT);
    return shaderStages;
}


std::vector<VkPipelineShaderStageCreateInfo> find_graphics_shader_module(const VK_backend &backend,
                                                                         const VKR_shader_paths &paths,
                                                                         const Shader_data &shader_data_handle) {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    create_Shader_Module(backend, shaderStages, paths.vertex_path_, shader_data_handle->spv_data_vert,
                         VK_SHADER_STAGE_VERTEX_BIT);
    create_Shader_Module(backend, shaderStages, paths.fragment_path_, shader_data_handle->spv_data_frag,
                         VK_SHADER_STAGE_FRAGMENT_BIT);
    create_Shader_Module(backend, shaderStages, paths.geometry_path_, shader_data_handle->spv_data_geo,
                         VK_SHADER_STAGE_GEOMETRY_BIT);
    return shaderStages;
}

//
// Created by 潘鑫 on 2026/6/2.
//

#include "shader_resolve.h"

#include "create_pipeline.h"
#include "descriptor_organized_sets_and_bindings.h"
#include "pipeline_layout.h"
#include "sets_and_bindings_layout.h"
#include "vulkan_backend.h"
#include "shader_create.h"

shader_data VKR_shader_init(VKR_shader_paths &shader_paths) {
    shader_data shader_data_handle;
    // if (shader_data_handle.get() == nullptr)
    // 这里需要进行检查,看看是否 存在相同的 VKR_shader_paths ,
    // 如果这里相同, 那么 后面的一切都是相同的
    {
        auto &backend = VK_backend::instance();
        shader_data_handle = std::make_shared<vk_shader_data>();
        shader_data_handle->pipeline_shader_stage_create_infos = find_graphics_shader_module(backend, shader_paths);
        shader_data_handle->computer_shader_stage_create_infos = find_compute_shader_module(backend, shader_paths);
        shader_data_handle->object_sets_bindings = organize_descriptor_set_and_binding_layouts(shader_paths,
                 shader_data_handle);
        shader_data_handle->shader_key = get_shader_key(shader_paths);
        shader_data_handle->topology   = shader_paths.topology_;
        // 下面这两个对于创建的顺序有点要求，上面的没有顺序要求

        // descriptor_sets_layout 中包含 global 的 set
        // 重要是如果有时候，set = 0 在 global 时应该如何处理


        shader_data_handle->bindless_set_layout =
                create_descriptor_sets_layout(backend,
                                              shader_data_handle->shader_key + "bindless_set",
                                              shader_data_handle->bindless_sets_bindings);
        shader_data_handle->global_descriptor_sets_layout =
                create_descriptor_sets_layout(backend,
                                              shader_data_handle->shader_key + "global_bindings_set",
                                              shader_data_handle->global_sets_bindings);

        shader_data_handle->object_descriptor_sets_layout =
                create_descriptor_sets_layout(backend,
                                              shader_data_handle->shader_key,
                                              shader_data_handle->object_sets_bindings);
        std::vector<VkDescriptorSetLayout> temp;
        temp.reserve(shader_data_handle->object_descriptor_sets_layout.size() +
                     shader_data_handle->bindless_set_layout.size() +
                     shader_data_handle->global_descriptor_sets_layout.size());

        temp.insert(temp.end(),
                    shader_data_handle->bindless_set_layout.begin(),
                    shader_data_handle->bindless_set_layout.end());
        temp.insert(temp.end(),
                    shader_data_handle->global_descriptor_sets_layout.begin(),
                    shader_data_handle->global_descriptor_sets_layout.end());
        temp.insert(temp.end(),
                    shader_data_handle->object_descriptor_sets_layout.begin(),
                    shader_data_handle->object_descriptor_sets_layout.end());


        shader_data_handle->pipeline_layout = create_pipeline_layout(backend, shader_data_handle->shader_key,
                                                                     temp, shader_data_handle->push_constant_map);

        shader_data_handle->pipeline_t = create_pipeline(backend, *shader_data_handle.get());
    }
    return shader_data_handle;
}


vk_shader_data::~vk_shader_data() {
    // 这里需要看看或者确定一下,有没有在管线 还是使用的过程中就删除了
    auto &handle = VK_backend::instance();
    if (pipeline_t != VK_NULL_HANDLE) {
        vkDestroyPipeline(handle.get_device(), pipeline_t, nullptr);
    }
    if (pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(handle.get_device(), pipeline_layout, nullptr);

    for (auto pipeline_shader_stage_create_info: pipeline_shader_stage_create_infos) {
        if (pipeline_shader_stage_create_info.module != VK_NULL_HANDLE)
            vkDestroyShaderModule(handle.get_device(), pipeline_shader_stage_create_info.module, nullptr);
    }
    for (auto pipeline_shader_stage_create_info: computer_shader_stage_create_infos) {
        if (pipeline_shader_stage_create_info.module != VK_NULL_HANDLE)
            vkDestroyShaderModule(handle.get_device(), pipeline_shader_stage_create_info.module, nullptr);
    }
    for (const auto set_layout: bindless_set_layout) {
        if (set_layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(handle.get_device(), set_layout, nullptr);
    }
    for (const auto set_layout: global_descriptor_sets_layout) {
        if (set_layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(handle.get_device(), set_layout, nullptr);
    }
    for (const auto set_layout: object_descriptor_sets_layout) {
        if (set_layout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(handle.get_device(), set_layout, nullptr);
    }
}

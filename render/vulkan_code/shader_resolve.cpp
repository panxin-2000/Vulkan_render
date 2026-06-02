//
// Created by 潘鑫 on 2026/6/2.
//

#include "shader_resolve.h"

#include "descriptor_organized_sets_and_bindings.h"
#include "pipeline_layout.h"
#include "sets_and_bindings_layout.h"
#include "vulkan_backend.h"
#include "shader_create.h"

shader_data VKR_shader_init(VKR_shader_paths &shader_paths) {
    shader_data shader_data_handle;
    // if (shader_data_handle.get() == nullptr)
    {
        auto &handle = VK_backend::instance();
        shader_data_handle = std::make_shared<vk_shader_data>();
        shader_data_handle->pipeline_shader_stage_create_infos = find_graphics_shader_module(handle, shader_paths);
        shader_data_handle->computer_shader_stage_create_infos = find_compute_shader_module(handle, shader_paths);
        shader_data_handle->object_sets_bindings = organize_descriptor_set_and_binding_layouts(shader_paths,
                 shader_data_handle);
        shader_data_handle->shader_key = get_shader_key(shader_paths);
        shader_data_handle->topology   = shader_paths.topology_;
        // 下面这两个对于创建的顺序有点要求，上面的没有顺序要求

        // descriptor_sets_layout 中包含 global 的 set
        // 重要是如果有时候，set = 0 在 global 时应该如何处理


        shader_data_handle->bindless_set_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key + "bindless_set",
                                              shader_data_handle->bindless_sets_bindings);
        shader_data_handle->global_descriptor_sets_layout =
                create_descriptor_sets_layout(handle,
                                              shader_data_handle->shader_key + "global_bindings_set",
                                              shader_data_handle->global_sets_bindings);

        shader_data_handle->object_descriptor_sets_layout =
                create_descriptor_sets_layout(handle,
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


        shader_data_handle->pipeline_layout = create_pipeline_layout(handle, shader_data_handle->shader_key,
                                                                     temp, shader_data_handle->push_constant_map);
    }
    return shader_data_handle;
}

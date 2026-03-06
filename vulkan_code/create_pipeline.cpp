//
// Created by 潘鑫 on 2026/3/4.
//
#include "create_pipeline.h"

#include "shader_common.h"
std::map<std::string, pipeline_and_share> pipeline_map_;

auto &get_pipeline_map() {
    return pipeline_map_;
}


VkPipeline create_pipeline(VK_handle &handle, vk_shader_data &data) {
    std::map<std::string, pipeline_and_share> &map = get_pipeline_map();
    if (!data.shader_key.empty()) {
        auto it = map.find(data.shader_key);
        if (it != map.end()) {
            it->second.shared_number++;
            return it->second.pipeline;
        } else {
            auto pipeline = create_graphics_pipeline(handle, data);
            map.insert({data.shader_key, {pipeline, 1}});
            return pipeline;
        }
    }
    return VK_NULL_HANDLE;
}

VkPipeline find_pipeline(VK_handle &handle, std::shared_ptr<vk_shader_data> &data) {
    std::map<std::string, pipeline_and_share> &map = get_pipeline_map();
    if (!data->shader_key.empty()) {
        auto it = map.find(data->shader_key);
        if (it != map.end()) {
            return it->second.pipeline;
        } else {
            return create_pipeline(handle, *data);
        }
    }
    return VK_NULL_HANDLE;
}


void clean_all_pipeline(VK_handle &handle) {
    auto pipeline_map = get_pipeline_map();
    for (const auto &[key, value]: pipeline_map) {
        vkDestroyPipeline(handle.get_device(), value.pipeline, nullptr);
    }
    pipeline_map.clear();
}

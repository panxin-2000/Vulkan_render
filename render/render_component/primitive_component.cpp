//
// Created by 潘鑫 on 2026/11/16.
//
#include "primitive_component.h"


std::vector<VKR_Primitive> create_primitives(const Geometry_data &data) {
    std::vector<VKR_Primitive> primitives;
    const std::vector<share_block> &vertices = data.get_vertices();
    const std::vector<share_block> &indices  = data.get_indices();
    if (!indices.empty() && indices.size() == vertices.size()) {
        size_t vertices_offset = 0;
        uint32_t first_index   = 0;
        for (uint32_t i = 0; i < indices.size(); i++) {
            const auto index  = indices[i];
            const auto vertex = vertices[i];
            VKR_Primitive primitive;
            primitive.vertexOffset = static_cast<int32_t>(vertices_offset); // 应该是这里的问题
            vertices_offset        += vertex.count;
            primitive.firstIndex   = first_index;
            first_index            += index.count;

            // 这里应该是没有什么问题的
            primitive.indexCount = index.count;
            //确实是可以通过计算偏移的
            primitive.instanceCount = 1; // 也就是这两个是需要去手动进行计算的
            primitive.firstInstance = 0; // 这里主要是为了进行bindless 相关的填充
            primitives.push_back(primitive);
        }
    } else {
        assert(false && " not deal indices empty");
    }
    for (uint32_t i = 0; i < primitives.size(); ++i) {
        primitives.at(i).firstInstance = i;
    }
    return primitives;
}


std::vector<VKR_Primitive> create_primitives(const entt::entity entity) {
    if (const auto data = Logic_entt().try_get<Geometry_data>(entity)) {
        return create_primitives(*data);
    }
    return {};
}

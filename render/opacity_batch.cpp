//
// Created by 潘鑫 on 2026/9/23.
//

#include "opacity_batch.h"

#include "render_state.h"
#include "VCB_vulkan_command_buffer.h"


void opacity_batch::check_and_update(sets_map &sets_map_in_for,
                                     std::map<std::string, Update_descriptor_binding> &update_descriptor_write) {
    auto view = Render_entt().view<Geometry_data, Mesh_need_create_tag,
                                   std::vector<World_Space_AABB>,
                                   std::vector<Transform_Matrix>,
                                   Gltf_material_parameters,
                                   screen_pick_entity>();

    bool view_has_value = false;
    for (const auto entity: view) {
        const auto geometry_data = Render_entt().get<Geometry_data>(entity);
        const auto vertices      = geometry_data.get_vertices();
        const auto indices       = geometry_data.get_indices();
        for (const auto &vertex: vertices) {
            bindless_Geometry_data_.push_vertices(vertex);
        }
        for (const auto &index: indices) {
            bindless_Geometry_data_.push_indices(index);
        }
        view_has_value = true;
        Render_entt().remove<Mesh_need_create_tag>(entity);
    }

    size_t primitive_size = 0;

    auto view_2 = Render_entt().view<Geometry_data,
                                     std::vector<World_Space_AABB>,
                                     std::vector<Transform_Matrix>,
                                     Gltf_material_parameters,
                                     screen_pick_entity>();


    for (const auto entity: view_2) {
        auto size      = Render_entt().get<std::vector<Transform_Matrix> >(entity).size();
        primitive_size += size;
    }

    if (primitive_size > 0 && view_has_value == true) {
        mRenderableData_.clear();
        mRenderableData_.setCapacity(ALIGN_16(primitive_size));


        for (const auto entity: view_2) {
            auto size = Render_entt().get<std::vector<Transform_Matrix> >(entity).size();
            for (uint i = 0; i < size; ++i) {
                auto soa_size = mRenderableData_.size();
                mRenderableData_.push_back();
                mRenderableData_.elementAt<SOA_World_Space_AABB>(soa_size) =
                        Render_entt().get<std::vector<World_Space_AABB> >(entity).at(i);
                mRenderableData_.elementAt<SOA_Transform_Matrix>(soa_size) =
                        Render_entt().get<std::vector<Transform_Matrix> >(entity).at(i);
                mRenderableData_.elementAt<SOA_material_index>(soa_size) =
                        Render_entt().get<Gltf_material_parameters>(entity).at(i);
                mRenderableData_.elementAt<SOA_entity>(soa_size) =
                        Render_entt().get<screen_pick_entity>(entity).at(i);
            }
        }


        if (view_has_value == true) {
            mesh_data_      = create_mesh_data(bindless_Geometry_data_);
            auto primitives = create_primitives(bindless_Geometry_data_);
            if (primitives.size() == mRenderableData_.size()) {
                for (size_t i = 0; i < mRenderableData_.size(); ++i) {
                    mRenderableData_.elementAt<SOA_Primitive>(i) = primitives.at(i);
                }
            }
        }
    }


    // 没有一套完整的, 可以解决全部的办法

    if (primitive_size > 0) {
        auto buffer_size          = RenderableSoa::getNeededSize(mRenderableData_.capacity());
        SOA_buffer_               = create_SSBO_buffer(buffer_size, mRenderableData_.data<0>());
        auto vkr_primitive_buffer = VKR_buffer_offset{
            SOA_buffer_, mRenderableData_.capacity() * sizeof(VKR_Primitive),
            RenderableSoa::getOffset(SOA_Primitive, mRenderableData_.capacity())
        };
        auto world_space_aabb_buffer = VKR_buffer_offset{
            SOA_buffer_, mRenderableData_.capacity() * sizeof(World_Space_AABB),
            RenderableSoa::getOffset(SOA_World_Space_AABB, mRenderableData_.capacity())
        };
        auto global_model_matrix = VKR_buffer_offset{
            SOA_buffer_, mRenderableData_.capacity() * sizeof(Transform_Matrix),
            RenderableSoa::getOffset(SOA_Transform_Matrix, mRenderableData_.capacity())
        };
        auto global_material_parameters = VKR_buffer_offset{
            SOA_buffer_, mRenderableData_.capacity() * sizeof(uint32_t),
            RenderableSoa::getOffset(SOA_material_index, mRenderableData_.capacity())
        };
        auto global_entity_to_screen = VKR_buffer_offset{
            SOA_buffer_, mRenderableData_.capacity() * sizeof(entt::entity),
            RenderableSoa::getOffset(SOA_entity, mRenderableData_.capacity())
        };

        VK_update_parameter(sets_map_in_for, update_descriptor_write,
                            "global_model_matrix", global_model_matrix);
        VK_update_parameter(sets_map_in_for, update_descriptor_write,
                            "global_entity_to_screen", global_entity_to_screen);
        VK_update_parameter(sets_map_in_for, update_descriptor_write,
                            "global_material_parameters", global_material_parameters);
    }
}

void opacity_batch::draw_depth_batch(VCB &vcb, const std::shared_ptr<vk_shader_data> &shader_data_ref) {
    if (mRenderableData_.size() > 0) {
        vcb.bind_pipeline_update_parameter(entt::null, shader_data_ref);
        // 现在绑定的管线是有问题的,
        vcb.default_status();
        // 想调用下面这个, 还需要汇总数据
        // vcb.DrawIndexedIndirect(entity, command_calculate.command_size, command_calculate.camera_write_buffer);
    }
}

void opacity_batch::draw_batch(VCB &vcb) {
    if (mRenderableData_.size() > 0) {
        auto frustum_culling_buffer = VKR_buffer_offset{
            SOA_buffer_,
            mRenderableData_.capacity() * sizeof(VKR_Primitive),
            RenderableSoa::getOffset(SOA_frustum_culling,
                                     mRenderableData_.capacity())
        };
        VKR_Render_state render_state; // 这个还算比较好解决
        vcb.DrawIndexedIndirect(mesh_data_,
                                render_state,
                                mRenderableData_.size(),
                                frustum_culling_buffer);
    }
}

void opacity_batch::draw_shadow(VCB &vcb) {
    if (mRenderableData_.size() > 0) {
        auto frustum_culling_buffer = VKR_buffer_offset{
            SOA_buffer_,
            mRenderableData_.capacity() * sizeof(VKR_Primitive),
            RenderableSoa::getOffset(SOA_shadow_frustum_culling,
                                     mRenderableData_.capacity())
        };
        VKR_Render_state render_state; // 这个还算比较好解决
        vcb.DrawIndexedIndirect(mesh_data_,
                                render_state,
                                mRenderableData_.size(),
                                frustum_culling_buffer);
    }
}

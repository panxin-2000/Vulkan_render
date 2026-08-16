//
// Created by 潘鑫 on 2026/8/13.
//


#include "GPU_frustum_cull.h"
#include "../engine.h"
#include "render_proxy.h"
#include "../render_common/render_state.h"
#include "../render_common/render_mesh.h"

#include "VCB_vulkan_command_buffer.h"
#include "sync_proxy_to_render_thread.h"

void VCB::draw(const Mesh_data &mesh_data,
               const std::vector<VKR_Primitive> &primitives,
               const std::vector<VKR_Render_state> *render_states) {
    if (mesh_data.vertices == nullptr || mesh_data.vertices->get_buffer_handle() == VK_NULL_HANDLE)
        return;
    // 这里有一个 可以优化的点 vkCmdBindVertexBuffers 的  vertices_offset
    // 和 draw_command.indexed_command.vertexOffset 如果设置这个,那么可以少绑定一次内容
    vkCmdBindVertexBuffers(command_buffer_, 0, 1,
                           mesh_data.vertices->get_buffer_handle_ptr(time_line_),
                           &mesh_data.vertices_offset);
    if (mesh_data.indices != nullptr &&
        mesh_data.indices->get_buffer_handle() != VK_NULL_HANDLE) {
        vkCmdBindIndexBuffer(command_buffer_,
                             mesh_data.indices->get_buffer_handle(),
                             mesh_data.indices_offset,
                             mesh_data.index_type);

        for (int i = 0; i < primitives.size(); ++i) {
            if (render_states != nullptr && primitives.size() == render_states->size()) {
                auto render_state = render_states->at(i);
                render_state.set_render_state_command(command_buffer_, VK_backend::instance().get_viewport(),
                                                      VK_backend::instance().get_scissor());
            }
            const auto primitive = primitives.at(i);
            // vertexOffset 只会影响最终传给顶点属性读取的顶点索引（即 Index + vertexOffset），
            // 它不会改变你用 vkCmdBindVertexBuffers 绑定的顶点缓冲区的内存起始地址
            vkCmdDrawIndexed(command_buffer_, primitive.indexCount,
                             primitive.instanceCount,
                             primitive.firstIndex,
                             primitive.vertexOffset,
                             primitive.firstInstance);
        }
    } else if (mesh_data.index_type == VK_INDEX_TYPE_MAX_ENUM) {
        assert(false && " not deal indices empty");
    }
}


void VCB::build_draw_command(entt::entity entity) {
    const auto &shader_data_ref = Render_entt().get<Shader_data>(entity);

    bind_pipeline_update_parameter(entity, shader_data_ref);

    const auto mesh_data     = Render_entt().get<Mesh_data>(entity);
    const auto primitives    = Render_entt().get<std::vector<VKR_Primitive> >(entity);
    const auto render_states = Render_entt().try_get<std::vector<VKR_Render_state> >(entity);

    if (!primitives.empty() && render_states != nullptr && !render_states->empty()) {
        draw(mesh_data, primitives, render_states);
    } else if (!primitives.empty() && render_states == nullptr) {
        constexpr VKR_Render_state temp;
        temp.set_render_state_command(command_buffer_, VK_backend::instance().get_viewport(),
                                      VK_backend::instance().get_scissor());
        draw(mesh_data, primitives, render_states);
    } else {
        // 为空并且有一个deferred 标记 // todo: 标记判断
        if (Render_entt().any_of<deferred_pass_tag>(entity))
            vkCmdDraw(command_buffer_, 3, 1, 0, 0);
        if (Render_entt().any_of<UI_2D_tag>(entity))
            vkCmdDraw(command_buffer_, 4, 1, 0, 0);
    }
}


void VCB::DrawIndexedIndirect(entt::entity entity,
                              GPU_frustum_cull command_calculate) {
    const auto mesh_data = Render_entt().get<Mesh_data>(entity);
    vkCmdBindVertexBuffers(command_buffer_, 0, 1,
                           mesh_data.vertices->get_buffer_handle_ptr(time_line_),
                           &mesh_data.vertices_offset);
    if (mesh_data.indices != nullptr &&
        mesh_data.indices->get_buffer_handle() != VK_NULL_HANDLE) {
        VKR_Render_state temp;
        temp.set_render_state_command(command_buffer_, VK_backend::instance().get_viewport(),
                                      VK_backend::instance().get_scissor());
        vkCmdBindIndexBuffer(command_buffer_,
                             mesh_data.indices->get_buffer_handle(),
                             mesh_data.indices_offset,
                             mesh_data.index_type);
        vkCmdDrawIndexedIndirect(command_buffer_,
                                 command_calculate.command_buffer->get_buffer_handle(time_line_),
                                 0,
                                 command_calculate.command_size,
                                 sizeof(VkDrawIndexedIndirectCommand));
    }
}

//
// Created by 潘鑫 on 2026/8/13.
//


#include "engine.h"
#include "render_proxy.h"
#include "../render_component/render_state_component.h"
#include "../render_component/primitive_component.h"

#include "VCB_vulkan_command_buffer.h"
#include "UI/UI_button.h"

void VCB::write_draw_command(const Mesh_data &mesh_data,
                             const std::vector<VKR_Primitive> &primitives,
                             const std::vector<VKR_Render_state> &render_states) const {
    if (mesh_data.write_bind_command(command_buffer_, time_line_) == true) {
        for (int i = 0; i < primitives.size(); ++i) {
            if (render_states.empty() == false && primitives.size() == render_states.size()) {
                render_states.at(i).write_commands(command_buffer_, pass_viewport_, pass_scissor_);
            } else if (render_states.size() == 1 && i == 0) {
                render_states.at(0).write_commands(command_buffer_, pass_viewport_, pass_scissor_);
            }
            primitives.at(i).write_commands(command_buffer_);
        }
    }
}


void VCB::draw_entity(const entt::entity entity) {
    const auto &shader_data_ref = Render_entt().get<Shader_data>(entity);

    bind_pipeline_update_parameter(entity, shader_data_ref);

    if (Render_entt().all_of<Mesh_data, std::vector<VKR_Primitive>, std::vector<VKR_Render_state> >(entity)) {
        const auto mesh_data     = Render_entt().get<Mesh_data>(entity);
        const auto primitives    = Render_entt().get<std::vector<VKR_Primitive> >(entity);
        const auto render_states = Render_entt().get<std::vector<VKR_Render_state> >(entity);
        write_draw_command(mesh_data, primitives, render_states); // 简单确定问题在这里了
    } else if (Render_entt().all_of<Mesh_data, std::vector<VKR_Primitive> >(entity) &&
               !Render_entt().any_of<std::vector<VKR_Render_state> >(entity)) {
        const auto mesh_data  = Render_entt().get<Mesh_data>(entity);
        const auto primitives = Render_entt().get<std::vector<VKR_Primitive> >(entity);
        constexpr VKR_Render_state temp;
        temp.write_commands(command_buffer_, pass_viewport_, pass_scissor_);
        write_draw_command(mesh_data, primitives, {});
    } else {
        // 为空并且有一个deferred 标记 // todo: 标记判断
        if (Render_entt().any_of<deferred_pass_tag>(entity))
            vkCmdDraw(command_buffer_, 3, 1, 0, 0);
        if (Render_entt().any_of<Tag_UI_Button>(entity))
            vkCmdDraw(command_buffer_, 4, 1, 0, 0);
    }
}

void VCB::build_draw_command_UI(const entt::entity entity) {
    const auto &shader_data_ref = Render_entt().get<Shader_data>(entity);

    bind_pipeline_update_parameter(entity, shader_data_ref);

    if (Render_entt().all_of<Mesh_data, std::vector<VKR_Primitive>, std::vector<VKR_Render_state> >(entity)) {
        const auto mesh_data     = Render_entt().get<Mesh_data>(entity);
        const auto primitives    = Render_entt().get<std::vector<VKR_Primitive> >(entity);
        const auto render_states = Render_entt().get<std::vector<VKR_Render_state> >(entity);
        write_draw_command(mesh_data, primitives, render_states);
    } else if (Render_entt().all_of<Mesh_data, std::vector<VKR_Primitive> >(entity) &&
               !Render_entt().any_of<std::vector<VKR_Render_state> >(entity)) {
        const auto mesh_data  = Render_entt().get<Mesh_data>(entity);
        const auto primitives = Render_entt().get<std::vector<VKR_Primitive> >(entity);
        VKR_Render_state temp;
        temp.set_depthTest_disable();
        temp.write_commands(command_buffer_, pass_viewport_, pass_scissor_);
        write_draw_command(mesh_data, primitives, {});
    } else {
        // 为空并且有一个deferred 标记 // todo: 标记判断
        if (Render_entt().any_of<deferred_pass_tag>(entity))
            vkCmdDraw(command_buffer_, 3, 1, 0, 0);
        if (Render_entt().any_of<Tag_UI_Button>(entity))
            vkCmdDraw(command_buffer_, 4, 1, 0, 0);
    }
}


void VCB::DrawIndexedIndirect(const entt::entity entity,
                              const uint32_t command_size,
                              const VKR_buffer_block &read_buffer) const {
    const auto mesh_data = Render_entt().get<Mesh_data>(entity);
    if (mesh_data.write_bind_command(command_buffer_, time_line_) == true) {
        VKR_Render_state temp;
        temp.write_commands(command_buffer_, pass_viewport_, pass_scissor_);
        vkCmdDrawIndexedIndirect(command_buffer_,
                                 read_buffer.get_buffer_handle(time_line_),
                                 read_buffer.get_offset(),
                                 command_size,
                                 sizeof(VkDrawIndexedIndirectCommand));
    }
}

void VCB::DrawIndexedIndirect(const Mesh_data &mesh_data,
                              const VKR_Render_state &render_state,
                              const uint32_t command_size,
                              const VKR_buffer_block &read_buffer) const {
    if (mesh_data.write_bind_command(command_buffer_, time_line_) == true) {
        render_state.write_commands(command_buffer_, pass_viewport_, pass_scissor_);
        vkCmdDrawIndexedIndirect(command_buffer_,
                                 read_buffer.get_buffer_handle(time_line_),
                                 read_buffer.get_offset(),
                                 command_size,
                                 sizeof(VkDrawIndexedIndirectCommand));
    }
}


// 这里是什么意思呢?  多批次物体剔除或大规模场景渲染 , 不是一个 批次 就完成了的
void VCB::DrawIndexedIndirectCount(const Mesh_data &mesh_data,
                                   const VKR_Render_state &render_state,
                                   const VKR_buffer_block &read_buffer,
                                   const VKR_buffer_block &countBuffer) const {
    if (mesh_data.write_bind_command(command_buffer_, time_line_) == true) {
        render_state.write_commands(command_buffer_, pass_viewport_, pass_scissor_);
        vkCmdDrawIndexedIndirectCount(
                                      command_buffer_,
                                      read_buffer.get_buffer_handle(time_line_), // 包含 draw_commands[] 的缓冲区
                                      read_buffer.get_offset(),                  // 数组起始偏移
                                      countBuffer.get_buffer_handle(time_line_), // 包含 actual_draw_count 的缓冲区
                                      countBuffer.get_offset(),                  // 计数器在 buffer 中的偏移
                                      countBuffer.get_size() / sizeof(VkDrawIndexedIndirectCommand),
                                      // 允许的最大批次数量（通常是你的场景最大物体数，防止 GPU 越界）
                                      sizeof(VkDrawIndexedIndirectCommand) // 每个命令的步长
                                     );
    }
}

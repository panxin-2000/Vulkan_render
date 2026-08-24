//
// Created by 潘鑫 on 2026/1/28.
//

#ifndef HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#define HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H
#include "GPU_frustum_cull.h"
#include "render_mesh.h"
#include "render_state.h"
#include "../engine.h"

struct G_buffer_image_index {
    uint32_t position_image_index;
    uint32_t normal_image_index;
    uint32_t baseColor_image_index;
};


class VCB {
    VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
    uint64_t time_line_             = 0;
    VkQueryPool query_pool_         = VK_NULL_HANDLE;

public:
    struct scoped_debug_label {
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;

        scoped_debug_label(VCB &vcb, const std::string &label) {
            command_buffer = vcb.command_buffer_;
            VkDebugUtilsLabelEXT labelInfo{};
            labelInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            labelInfo.pLabelName = label.c_str();
            labelInfo.color[0]   = 1.0f; // R (0.0~1.0)
            labelInfo.color[1]   = 1.0f; // G
            labelInfo.color[2]   = 0.0f; // B (黄色)
            labelInfo.color[3]   = 1.0f; // A
            vkCmdBeginDebugUtilsLabelEXT(command_buffer, &labelInfo);
        };

        ~scoped_debug_label() {
            vkCmdEndDebugUtilsLabelEXT(command_buffer);
        };
    };

    void reset_current_command_buffer(uint64_t time_line, VkCommandBuffer);

    void end_rendering();

    void submit_render_queue(Engine &engine);

    void end_command_buffer();


    G_buffer_image_index begin_g_buffer_rendering_attachment(
        const std::vector<VKR_image_ptr> &color,
        const VKR_image_ptr &depth, VkAttachmentLoadOp depth_loadOp);

    void current_write_next_read_depth(
        const std::vector<VKR_image_ptr> &images);

    void current_write_next_read_image(
        const std::vector<VKR_image_ptr> &images);

    void begin_shadow_pass(VKR_image_ptr depth_image);

    void shadow_pass_barrier();

    void build_draw_command(entt::entity entity);


    void render_post_deal(std::shared_ptr<vk_shader_data> command_shader, entt::entity entity) {
        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, command_shader->pipeline_t);
        bind_Proxy_descriptor_sets(entity,
                                   command_shader->pipeline_layout,
                                   VK_PIPELINE_BIND_POINT_GRAPHICS);
        constexpr VKR_Render_state temp;
        temp.set_render_state_command(command_buffer_, VK_backend::instance().get_viewport(),
                                      VK_backend::instance().get_scissor());
        vkCmdSetCullMode(command_buffer_, VK_CULL_MODE_NONE);
        vkCmdSetDepthTestEnable(command_buffer_, VK_FALSE);

        vkCmdDraw(command_buffer_, 3, 1, 0, 0);
    }


    void DrawIndexedIndirect(entt::entity entity,
                             GPU_frustum_cull command_calculate);

    inline void draw(
        const Mesh_data &mesh_data,
        const std::vector<VKR_Primitive> &primitives,
        const std::vector<VKR_Render_state> *render_states);


    void begin_rendering_depth_attachment(
        VKR_image_ptr depth,
        VkAttachmentLoadOp depth_loadOp);


    void begin_rendering_attachment(VKR_image_ptr color, VKR_image_ptr depth,
                                    VkAttachmentLoadOp depth_loadOp);


    void add_one_indirect_draw_barrier(VkBuffer buffer, VkDeviceSize size,
                                       VkDeviceSize offset = 0);

    void build_compute_dispatch(entt::entity entity);


    void calculate_frustum_cull(
        const entt::entity entity,
        const FrustumPlanes &frustum_planes);

    void default_status() const {
        constexpr VKR_Render_state temp;
        temp.set_render_state_command(command_buffer_, VK_backend::instance().get_viewport(),
                                      VK_backend::instance().get_scissor());
    }

    void PushConstants(VkPipelineLayout layout,
                       VkShaderStageFlags stageFlags,
                       uint32_t offset,
                       uint32_t size,
                       const void *pValues) const {
        vkCmdPushConstants(command_buffer_, layout,
                           stageFlags, offset, size, pValues);
    }

    void bind_Proxy_descriptor_sets(entt::entity entity,
                                    VkPipelineLayout pipeline_layout,
                                    VkPipelineBindPoint bind_point);

    void bind_pipeline_update_parameter(entt::entity entity, const Shader_data &shader_data_ref);

    void gpu_log_label_info(const std::string &label) {
        VkDebugUtilsLabelEXT markerInfo{};
        markerInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        markerInfo.pLabelName = label.c_str();
        markerInfo.color[0]   = 1.0f; // R (0.0~1.0)
        markerInfo.color[1]   = 1.0f; // G
        markerInfo.color[2]   = 0.0f; // B (黄色)
        markerInfo.color[3]   = 1.0f; // A
        vkCmdInsertDebugUtilsLabelEXT(command_buffer_, &markerInfo);
    }

    void SetDebugName(const VK_backend &backend,
                      const VkObjectType objectType,
                      const uint64_t handle,
                      const std::string &name);
};


#endif //HELLO_MAC_VULKAN_BUILD_COMMAND_BUFFER_H

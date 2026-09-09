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
    VkViewport pass_viewport_       = {};
    VkRect2D pass_scissor_          = {};

public:
    VkCommandBuffer get_command_buffer() const {
        return command_buffer_;
    }

    VkQueryPool get_query_pool() const {
        return query_pool_;
    }


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

    void set_pass_viewport(const uint32_t width, const uint32_t height) {
        pass_viewport_ = VkViewport{
            .x        = 0,
            .y        = 0,
            .width    = static_cast<float>(width),
            .height   = static_cast<float>(height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
    }

    void set_pass_scissor(const uint32_t width, const uint32_t height) {
        pass_scissor_ = VkRect2D{
            .offset = {0, 0},
            .extent = {width, height},
        };
    }

    void reset_current_command_buffer(uint64_t time_line, VkCommandBuffer);

    void end_rendering();

    void submit_render_queue(Engine &engine);

    void compute_write_finish_barrier(const VKR_image_ptr &compute_write_finish_image);

    void compute_write_finish_same_read(const VKR_image_ptr &compute_write_finish_image);

    void compute_write_finish_sample_read(const VKR_image_ptr &compute_write_finish_image);

    void dof_blur(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void SSAO(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void blur_SSAO(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void only_image_compute(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image,
                            const std::string &compute_path);

    void down_sample(Engine &engine, VKR_image_ptr image_ptr,
                     const std::string &compute_path);

    void up_sample(Engine &engine, VKR_image_ptr image_ptr, const std::string &compute_path);

    void tone_mapping(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void CAS(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void FSR1_EASU(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void FSR1_RCAS(Engine &engine, VKR_image_ptr input_image, VKR_image_ptr out_image);

    void dof_composite(Engine &engine, VKR_image_ptr dof_image, VKR_image_ptr color_image,
                       VKR_image_ptr compute_write_image);

#define  blank_stage VK_PIPELINE_STAGE_2_NONE,VK_ACCESS_2_NONE,VK_IMAGE_LAYOUT_UNDEFINED
#define  compute_write_image2D VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,VK_ACCESS_2_SHADER_WRITE_BIT,VK_IMAGE_LAYOUT_GENERAL
#define  compute_read_sampler2D VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,VK_ACCESS_2_SHADER_READ_BIT,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
#define  transfer_read_src VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_ACCESS_2_TRANSFER_READ_BIT,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
#define  transfer_write_dsr  VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_ACCESS_2_TRANSFER_WRITE_BIT,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
#define  fragment_read_sampler2d  VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,VK_ACCESS_2_SHADER_READ_BIT,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
#define  FRAGMENT_READ_sampler2D
    /**
     *  blank_stage / compute_write_image2D /
     * @param image
     * @param srcStageMask
     * @param srcAccessMask
     * @param oldLayout
     * @param dstStageMask
     * @param dstAccessMask
     * @param newLayout
     */
    void add_image_barrier(const VKR_image_ptr &image, VkPipelineStageFlags srcStageMask, VkAccessFlags srcAccessMask,
                           VkImageLayout oldLayout, VkPipelineStageFlags dstStageMask, VkAccessFlags dstAccessMask,
                           VkImageLayout newLayout,
                           uint32_t baseMipLevel   = 0,
                           uint32_t levelCount     = 1,
                           uint32_t baseArrayLayer = 0,
                           uint32_t layerCount     = 1
    );

    void compute_write_init_barrier(const VKR_image_ptr &compute_write_finish_image);

    void end_command_buffer();


    G_buffer_image_index begin_g_buffer_rendering_attachment(
        const std::vector<VKR_image_ptr> &color,
        const VKR_image_ptr &depth, VkAttachmentLoadOp depth_loadOp);

    void current_write_next_read_depth(
        const std::vector<VKR_image_ptr> &images);

    void current_write_next_read_image(
        const std::vector<VKR_image_ptr> &images);

    void shadow_pass_barrier();

    void build_draw_command(entt::entity entity);

    void build_draw_command_UI(entt::entity entity);


    void render_post_deal(std::shared_ptr<vk_shader_data> command_shader, entt::entity entity) {
        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, command_shader->pipeline_t);
        bind_Proxy_descriptor_sets(entity,
                                   command_shader->pipeline_layout,
                                   VK_PIPELINE_BIND_POINT_GRAPHICS);
        constexpr VKR_Render_state temp;
        temp.set_render_state_command(command_buffer_, pass_viewport_, pass_scissor_);
        vkCmdSetCullMode(command_buffer_, VK_CULL_MODE_NONE);
        vkCmdSetDepthTestEnable(command_buffer_, VK_FALSE);

        vkCmdDraw(command_buffer_, 3, 1, 0, 0);
    }


    void DrawIndexedIndirect(entt::entity entity,
                             uint32_t command_size, VKR_buffer_ptr read_buffer);

    inline void draw(
        const Mesh_data &mesh_data,
        const std::vector<VKR_Primitive> &primitives,
        const std::vector<VKR_Render_state> *render_states);


    void begin_rendering_depth_attachment(
        VKR_image_ptr depth,
        VkAttachmentLoadOp depth_loadOp);

    void begin_rendering_attachment_to_screen(VKR_image_ptr color,
                                              VkAttachmentLoadOp color_loadOp);


    void begin_rendering_attachment(VKR_image_ptr color, VKR_image_ptr depth,
                                    VkAttachmentLoadOp depth_loadOp);


    void add_one_indirect_draw_barrier(VkBuffer buffer, VkDeviceSize size,
                                       VkDeviceSize offset = 0);

    void build_compute_dispatch(entt::entity entity);


    void render_3DGS_preprocess(entt::entity entity);

    VKR_buffer_ptr render_3DGS_prefixsum(entt::entity entity);

    void render_3DGS_idkeys(entt::entity entity, const VKR_buffer_ptr &prefix_sum);

    void render_3DGS_histogram_radixsort(entt::entity entity, VKR_buffer_ptr keys,
                                         VKR_buffer_ptr histograms,
                                         VKR_buffer_ptr keysRadix,
                                         VKR_buffer_ptr values,
                                         VKR_buffer_ptr valuesRadix
    );

    void render_3DGS_tile_boundaries(entt::entity entity);

    void render_3DGS_render(entt::entity entity);

    void copy_image(VKR_image_ptr src_image, VKR_image_ptr dst_image);

    void deal_image(entt::entity entity, const VKR_image_ptr &write,
                    const std::shared_ptr<vk_shader_data> &shader_data_ref);

    void add_buffer_read_to_write_barriers(const std::vector<VKR_buffer_ptr> &buffer_ptrs) const;

    void add_buffer_write_to_read_barriers(const std::vector<VKR_buffer_ptr> &buffer_ptrs) const;

    void calculate_frustum_cull(
        const entt::entity entity,
        const FrustumPlanes &frustum_planes,
        const std::array<FrustumPlanes, 4> &light_frustum_planes);

    void default_status() const {
        constexpr VKR_Render_state temp;
        temp.set_render_state_command(command_buffer_, pass_viewport_, pass_scissor_);
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

    void bind_Proxy_descriptor_sets(Proxy_descriptor_sets vk_descriptor_sets,
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

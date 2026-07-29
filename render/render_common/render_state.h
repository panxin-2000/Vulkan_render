//
// Created by 潘鑫 on 2026/7/28.
//

#ifndef HELLO_MAC_RENDER_STATE_H
#define HELLO_MAC_RENDER_STATE_H
#include <volk.h>
#include "vulkan_buffer.h"
#include "vulkan_pipeline_dynamic_state.h"

class VKR_Render_state {
public:
    VkViewport viewport = {0, 0, 0, 0, 0, 0};
    VkRect2D scissor    = {0, 0, 0, 0};
    PipelineRasterizationState pipelineRasterizationState;
    PipelineDynamicState pipelineDynamicState;

    /**
     *
     * @param frontFace VK_FRONT_FACE_COUNTER_CLOCKWISE / VK_FRONT_FACE_CLOCKWISE
     */
    void set_front_face(const VkFrontFace frontFace) {
        pipelineRasterizationState.frontFace_ = frontFace;
    }

    /**
     *
     * @param cullMode VK_CULL_MODE_BACK_BIT / VK_CULL_MODE_FRONT_BIT / VK_CULL_MODE_FRONT_AND_BACK / VK_CULL_MODE_NONE
     */
    void set_VkCullModeFlags(const VkCullModeFlags cullMode) {
        pipelineRasterizationState.cullMode_ = cullMode;
    }

    void set_render_state_command(const VkCommandBuffer &cb,
                                  VkViewport global_viewport,
                                  VkRect2D global_scissor) const {
        if (viewport.x == 0 && viewport.y == 0 &&
            viewport.width == 0 && viewport.height == 0 &&
            viewport.minDepth == 0 && viewport.maxDepth == 0) {
            vkCmdSetViewport(cb, 0, 1, &global_viewport);
        } else {
            vkCmdSetViewport(cb, 0, 1, &viewport);
        }
        if (scissor.extent.width == 0 && scissor.extent.height == 0 &&
            scissor.offset.x == 0 && scissor.offset.y == 0) {
            vkCmdSetScissor(cb, 0, 1, &global_scissor);
        } else {
            vkCmdSetScissor(cb, 0, 1, &scissor);
        }

        pipelineDynamicState.write_commands(cb);
        pipelineRasterizationState.write_commands(cb);
    }
};


#endif //HELLO_MAC_RENDER_STATE_H

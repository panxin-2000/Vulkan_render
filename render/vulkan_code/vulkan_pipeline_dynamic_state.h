//
// Created by 潘鑫 on 2026/3/23.
//

#ifndef HELLO_MAC_VULKAN_PIPELINE_DYNAMIC_STATE_H
#define HELLO_MAC_VULKAN_PIPELINE_DYNAMIC_STATE_H

#include <volk.h>

struct PipelineDynamicState {
    // VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,        // vkCmdSetDepthTestEnable
    // VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,       // vkCmdSetDepthWriteEnable
    // VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,         // vkCmdSetDepthCompareOp
    // VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE, // vkCmdSetDepthBoundsTestEnable
    // VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,      // vkCmdSetStencilTestEnable
    // VK_DYNAMIC_STATE_STENCIL_OP,               // vkCmdSetStencilOp
    // VK_DYNAMIC_STATE_DEPTH_BOUNDS,             // vkCmdSetDepthBounds
    // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK
    // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK
    // VK_DYNAMIC_STATE_STENCIL_REFERENCE


    VkBool32 depthTestEnable   = VK_TRUE;                     // VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE
    VkBool32 depthWriteEnable  = VK_TRUE;                     // VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // VK_DYNAMIC_STATE_DEPTH_COMPARE_OP

    VkBool32 depthBoundsTestEnable = VK_FALSE; // VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE
    float minDepthBounds           = 0.0f;     // VK_DYNAMIC_STATE_DEPTH_BOUNDS
    float maxDepthBounds           = 1.0f;     // VK_DYNAMIC_STATE_DEPTH_BOUNDS

    VkBool32 stencilTestEnable = VK_FALSE; // VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE
    VkStencilOpState front{
        .failOp      = VK_STENCIL_OP_ZERO,   // 模板测试失败时执行的操作
        .passOp      = VK_STENCIL_OP_KEEP,   // 模板测试和深度测试都通过时执行的操作
        .depthFailOp = VK_STENCIL_OP_KEEP,   // 模板测试通过但深度测试失败时执行的操作
        .compareOp   = VK_COMPARE_OP_ALWAYS, // 比较函数（如：VK_COMPARE_OP_EQUAL）
        .compareMask = 0xff,                 // 参与比较的位掩码
        .writeMask   = 0xff,                 // 允许写入模板缓冲的位掩码
        .reference   = 0xff,                 // 用于比较的参考值
    };                                       // VK_DYNAMIC_STATE_STENCIL_OP
    VkStencilOpState back{
        .failOp      = VK_STENCIL_OP_ZERO,
        .passOp      = VK_STENCIL_OP_KEEP,
        .depthFailOp = VK_STENCIL_OP_KEEP,
        .compareOp   = VK_COMPARE_OP_ALWAYS,
        .compareMask = 0xff,
        .writeMask   = 0xff,
        .reference   = 0xff,
    };
    //     常见应用：物体描边 (Outlining)
    // 如果你想给物体画个轮廓，通常会这样配置 front：
    // 第一步（绘制物体）：将 compareOp 设为 VK_COMPARE_OP_ALWAYS，
    //                     passOp 设为 VK_STENCIL_OP_REPLACE，
    //                     并将 reference 设为 1。这样物体覆盖的区域模板值都会变成 1。
    // 第二步（绘制放大后的轮廓）：将 compareOp 设为 VK_COMPARE_OP_NOT_EQUAL，reference 保持 1。这样只有在物体之外的像素才会通过测试并绘制。

    void write_commands(const VkCommandBuffer cb) const {
        if (vkCmdSetDepthTestEnable(cb, depthTestEnable),
            vkCmdSetDepthWriteEnable(cb, depthWriteEnable);
            depthTestEnable) {
            vkCmdSetDepthCompareOp(cb, depthCompareOp);
            if (vkCmdSetDepthBoundsTestEnable(cb, depthBoundsTestEnable); depthBoundsTestEnable)
                vkCmdSetDepthBounds(cb, minDepthBounds, maxDepthBounds);
        }
        if (vkCmdSetStencilTestEnable(cb, stencilTestEnable); stencilTestEnable) {
            vkCmdSetStencilOp(cb, VK_STENCIL_FACE_FRONT_BIT,
                              front.failOp,
                              front.passOp,
                              front.depthFailOp,
                              front.compareOp);
            vkCmdSetStencilCompareMask(cb, VK_STENCIL_FACE_FRONT_BIT, front.compareMask);
            vkCmdSetStencilWriteMask(cb, VK_STENCIL_FACE_FRONT_BIT, front.writeMask);
            vkCmdSetStencilReference(cb, VK_STENCIL_FACE_FRONT_BIT, front.reference);
            vkCmdSetStencilOp(cb, VK_STENCIL_FACE_BACK_BIT,
                              back.failOp,
                              back.passOp,
                              back.depthFailOp,
                              back.compareOp);
            vkCmdSetStencilCompareMask(cb, VK_STENCIL_FACE_BACK_BIT, back.compareMask);
            vkCmdSetStencilWriteMask(cb, VK_STENCIL_FACE_BACK_BIT, back.writeMask);
            vkCmdSetStencilReference(cb, VK_STENCIL_FACE_BACK_BIT, back.reference);
        }
    }
};


struct PipelineRasterizationState {
    VkCullModeFlags cullMode_     = VK_CULL_MODE_BACK_BIT;           // VK_DYNAMIC_STATE_CULL_MODE
    VkFrontFace frontFace_        = VK_FRONT_FACE_COUNTER_CLOCKWISE; // VK_DYNAMIC_STATE_FRONT_FACE
    VkBool32 depthBiasEnable      = VK_FALSE;                        // VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE
    float depthBiasConstantFactor = 0.0f;                            // VK_DYNAMIC_STATE_DEPTH_BIAS
    float depthBiasClamp          = 0.0f;
    float depthBiasSlopeFactor    = 0.0f;
    // VkPolygonMode polygonMode     = VK_POLYGON_MODE_FILL; // 这里没有
    bool vulkan_y_flip = false; // 默认在vulkan上的 对y轴进行了翻转 // 然后 最后的投影矩阵 其实又反转了一次

    void write_commands(const VkCommandBuffer cb) const {
        if (vulkan_y_flip == true) {
            if (VK_FRONT_FACE_COUNTER_CLOCKWISE == frontFace_)
                vkCmdSetFrontFace(cb, VK_FRONT_FACE_CLOCKWISE);
            else if (VK_FRONT_FACE_CLOCKWISE == frontFace_)
                vkCmdSetFrontFace(cb, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        } else {
            vkCmdSetFrontFace(cb, frontFace_);
        }
        vkCmdSetCullMode(cb, cullMode_);

        vkCmdSetFrontFace(cb, frontFace_);
        vkCmdSetCullMode(cb, cullMode_);
        // vkCmdSetPolygonModeEXT(cb, polygonMode);
        if (vkCmdSetDepthBiasEnable(cb, depthBiasEnable); depthBiasEnable)
            vkCmdSetDepthBias(cb, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
};


#endif //HELLO_MAC_VULKAN_PIPELINE_DYNAMIC_STATE_H

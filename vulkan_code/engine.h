//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H


#include "vulkan_device_handle.h"

#include "descriptor_pool.h"


struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model[3];
    glm::vec4 lightPos{0.0f, -10.0f, 10.0f, 0.0f};
    uint32_t selected{1};
} shaderData{};

struct ShaderDataBuffer {
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceAddress deviceAddress{};
    void *mapped{nullptr};
};


class Engine {
    VKDevice &handle_;
    VkCommandPool commandPool{VK_NULL_HANDLE};
    std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;
    std::array<ShaderDataBuffer, maxFramesInFlight> shaderDataBuffers;
    std::array<VkFence, maxFramesInFlight> fences;
    std::array<VkSemaphore, maxFramesInFlight> presentSemaphores;
    std::vector<VkSemaphore> render_to_image_semaphores_;

    /**
     * frameIndex 正在渲染的一帧图像
     * imageIndex swap chain 创建的 image 的索引
     * 可能会有三个 image 交替显示到屏幕
     * 但是永远只有 一个 image 用于渲染
     * 屏幕绘制比较快的话，三缓冲没有太大的作用
    * 屏幕绘制比较慢的话，丢弃过时帧，选择最新帧绘制，开始渲染到开始显示的延迟的延迟不一致的问题
     */
    uint32_t frameIndex{0}; //

    uint32_t imageIndex{0};

public:
    Engine(VKDevice &handle) : handle_{handle} {
    }

    VKDevice &get_handle() {
        return handle_;
    }


    void init() {
        create_command_pool();
        create_command_buffer();
        create_shader_data_buffer();
        create_fences();
        create_present_Semaphores();
        create_renderSemaphores();
    }

    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences;
    }

    VkFence &get_current_fences() {
        return get_fences()[frameIndex];
    }

    std::array<VkSemaphore, maxFramesInFlight> &get_presentSemaphores() {
        return presentSemaphores;
    }

    VkSemaphore &get_current_presentSemaphores() {
        return get_presentSemaphores()[frameIndex];
    }

    std::vector<VkSemaphore> &get_can_render_to_image_semaphores() {
        return render_to_image_semaphores_;
    }

    VkSemaphore &get_current_renderSemaphores() {
        return get_can_render_to_image_semaphores()[frameIndex];
    }

    VkCommandPool &get_command_pool() {
        return commandPool;
    }

    std::array<VkCommandBuffer, maxFramesInFlight> &get_command_buffers() {
        return commandBuffers;
    }

    VkCommandBuffer &get_current_command_buffer() {
        return get_command_buffers()[frameIndex];
    }

    std::array<ShaderDataBuffer, maxFramesInFlight> &get_shader_data_buffer() {
        return shaderDataBuffers;
    }

    ShaderDataBuffer &get_current_shader_data_buffer() {
        return get_shader_data_buffer()[frameIndex];
    }


    const VkImage &get_current_swap_chain_image() {
        return handle_.get_swap_chain_images()[imageIndex];
    }

    const VkImageView &get_current_swap_image_view() {
        return handle_.get_swap_image_views()[imageIndex];
    }

    void create_command_buffer() {
        VkCommandBufferAllocateInfo cbAllocCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = maxFramesInFlight
        };
        VK_CHECK_RESULT(vkAllocateCommandBuffers(handle_.get_device(), &cbAllocCI, commandBuffers.data()));
    }

    void create_command_pool() {
        // Command pool
        VkCommandPoolCreateInfo commandPoolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = handle_.get_queue_Family()
        };
        VK_CHECK_RESULT(vkCreateCommandPool(handle_.get_device(), &commandPoolCI, nullptr, &commandPool));
    }

    void create_shader_data_buffer() {
        // Shader data buffers
        for (auto i = 0; i < maxFramesInFlight; i++) {
            VkBufferCreateInfo uBufferCI{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = sizeof(ShaderData),
                .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            };
            VmaAllocationCreateInfo uBufferAllocCI{
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                         VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                         VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO
            };
            VK_CHECK_RESULT(
                vmaCreateBuffer(handle_.get_allocator(), &uBufferCI, &uBufferAllocCI, &shaderDataBuffers[i].buffer,
                    &shaderDataBuffers[i].allocation, nullptr));
            VK_CHECK_RESULT(
                vmaMapMemory(handle_.get_allocator(), shaderDataBuffers[i].allocation, &shaderDataBuffers[i].mapped));
            VkBufferDeviceAddressInfo uBufferBdaInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = shaderDataBuffers[i].buffer
            };
            shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(handle_.get_device(), &uBufferBdaInfo);
        }
    }

    void create_fences() {
        VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        for (auto i = 0; i < maxFramesInFlight; i++) {
            VK_CHECK_RESULT(vkCreateFence(handle_.get_device(), &fenceCI, nullptr, &fences[i]));
        }
    }

    void create_present_Semaphores() {
        VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        for (auto i = 0; i < maxFramesInFlight; i++) {
            VK_CHECK_RESULT(vkCreateSemaphore(handle_.get_device(), &semaphoreCI, nullptr, &presentSemaphores[i]));
        }
    }

    void create_renderSemaphores() {
        VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        render_to_image_semaphores_.resize(handle_.get_swap_image_views().size());
        LOG_INFO(g_log(), "get_swap_image_view size :  {}!", render_to_image_semaphores_.size());
        for (auto &semaphore: render_to_image_semaphores_) {
            VK_CHECK_RESULT(vkCreateSemaphore(handle_.get_device(), &semaphoreCI, nullptr, &semaphore));
        }
    }

    void put_one_image_to_screen() {
        // Submit to graphics queue
        VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // 为了处理“交换链图像（Swapchain Image）还没准备好”的问题  图像还没有从显示器“拿回来”
        auto cb = get_current_command_buffer();
        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &get_current_presentSemaphores(),
            .pWaitDstStageMask = &waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &cb,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
        };
        VK_CHECK_RESULT(vkQueueSubmit(handle_.get_queue(), 1, &submitInfo, get_current_fences()));

        frameIndex = (frameIndex + 1) % maxFramesInFlight;
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &get_can_render_to_image_semaphores()[imageIndex], // 不需要++ ？？可以，
            .swapchainCount = 1,
            .pSwapchains = &handle_.get_swap_chain(),
            .pImageIndices = &imageIndex
        };
        auto result = vkQueuePresentKHR(handle_.get_queue(), &presentInfo);
        if (result == VK_SUCCESS) {
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || handle_.framebufferResized) {
            handle_.recreate_swap_chain();
            destroy_and_recreate_fence_and_semaphore();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cout << "failed to acquire swap chain image!" << std::endl;
        }
    }

    /**
     *
     * @param imageIndex 必须用 imageIndex 去找图像资源
     */
    void get_one_image_can_render() {
        // forces the CPU to stop and wait until the GPU has finished executing a specific batch of commands
        VK_CHECK_RESULT(vkWaitForFences(handle_.get_device(), 1, &get_current_fences(), true, UINT64_MAX));
        VK_CHECK_RESULT(vkResetFences(handle_.get_device(), 1, &get_current_fences()));
        auto result = vkAcquireNextImageKHR(handle_.get_device(),
                                            handle_.get_swap_chain(),
                                            UINT64_MAX,
                                            get_current_presentSemaphores(),
                                            VK_NULL_HANDLE,
                                            &imageIndex);
        if (result == VK_SUCCESS) {
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || handle_.framebufferResized) {
            handle_.recreate_swap_chain();
            destroy_and_recreate_fence_and_semaphore();
            VK_CHECK_RESULT(vkWaitForFences(handle_.get_device(), 1, &get_current_fences(), true, UINT64_MAX));
            VK_CHECK_RESULT(vkResetFences(handle_.get_device(), 1, &get_current_fences()));
            auto result = vkAcquireNextImageKHR(handle_.get_device(),
                                                handle_.get_swap_chain(),
                                                UINT64_MAX,
                                                get_current_presentSemaphores(),
                                                VK_NULL_HANDLE,
                                                &imageIndex);
            if (result == VK_SUCCESS) {
            } else {
                std::cout << "failed to acquire swap chain image after recreate!" << std::endl;
                exit(0);
            }
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cout << "failed to acquire swap chain image!" << std::endl;
        }
    }


    void destroy_and_recreate_fence_and_semaphore() {
        for (auto i = 0; i < maxFramesInFlight; i++) {
            vkDestroyFence(handle_.get_device(), fences[i], nullptr);                //  这里还需要
            vkDestroySemaphore(handle_.get_device(), presentSemaphores[i], nullptr); //
        }
        for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
            vkDestroySemaphore(handle_.get_device(), render_to_image_semaphores_[i], nullptr);
        }
        create_fences();
        create_present_Semaphores();
        create_renderSemaphores();
        imageIndex = 0;
        frameIndex = 0;
    }

    void destroy() {
        VK_CHECK_RESULT(vkDeviceWaitIdle(handle_.get_device()));
        for (auto i = 0; i < maxFramesInFlight; i++) {
            vkDestroyFence(handle_.get_device(), fences[i], nullptr);                //  这里还需要
            vkDestroySemaphore(handle_.get_device(), presentSemaphores[i], nullptr); //
            vmaUnmapMemory(handle_.get_allocator(), shaderDataBuffers[i].allocation);
            vmaDestroyBuffer(handle_.get_allocator(), shaderDataBuffers[i].buffer, shaderDataBuffers[i].allocation);
        }
        for (auto i = 0; i < render_to_image_semaphores_.size(); i++) {
            vkDestroySemaphore(handle_.get_device(), render_to_image_semaphores_[i], nullptr);
        }
    }
};


#endif //HELLO_MAC_ENGINE_H

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
    VKDevice *handle_;
    VkCommandPool commandPool{VK_NULL_HANDLE};
    std::array<VkCommandBuffer, maxFramesInFlight> commandBuffers;
    std::array<ShaderDataBuffer, maxFramesInFlight> shaderDataBuffers;
    std::array<VkFence, maxFramesInFlight> fences;
    std::array<VkSemaphore, maxFramesInFlight> presentSemaphores;
    std::vector<VkSemaphore> renderSemaphores;

public:
    Engine(VKDevice *handle) : handle_{handle} {
    }

    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences;
    }

    std::array<VkSemaphore, maxFramesInFlight> &get_presentSemaphores() {
        return presentSemaphores;
    }

    std::vector<VkSemaphore> &get_renderSemaphores() {
        return renderSemaphores;
    }

    VkCommandPool &get_command_pool() {
        return commandPool;
    }

    std::array<VkCommandBuffer, maxFramesInFlight> &get_command_buffers() {
        return commandBuffers;
    }

    std::array<ShaderDataBuffer, maxFramesInFlight> &get_shader_data_buffer() {
        return shaderDataBuffers;
    }

    void create_command_buffer() {
        VkCommandBufferAllocateInfo cbAllocCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = maxFramesInFlight
        };
        VK_CHECK_RESULT(vkAllocateCommandBuffers(handle_->get_device(), &cbAllocCI, commandBuffers.data()));
    }

    void create_command_pool() {
        // Command pool
        VkCommandPoolCreateInfo commandPoolCI{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = handle_->get_queue_Family()
        };
        VK_CHECK_RESULT(vkCreateCommandPool(handle_->get_device(), &commandPoolCI, nullptr, &commandPool));
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
                vmaCreateBuffer(handle_->get_allocator(), &uBufferCI, &uBufferAllocCI, &shaderDataBuffers[i].buffer,
                    &shaderDataBuffers[i].allocation, nullptr));
            VK_CHECK_RESULT(
                vmaMapMemory(handle_->get_allocator(), shaderDataBuffers[i].allocation, &shaderDataBuffers[i].mapped));
            VkBufferDeviceAddressInfo uBufferBdaInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = shaderDataBuffers[i].buffer
            };
            shaderDataBuffers[i].deviceAddress = vkGetBufferDeviceAddress(handle_->get_device(), &uBufferBdaInfo);
        }
    }

    void create_fences() {
        VkFenceCreateInfo fenceCI{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        for (auto i = 0; i < maxFramesInFlight; i++) {
            VK_CHECK_RESULT(vkCreateFence(handle_->get_device(), &fenceCI, nullptr, &fences[i]));
        }
    }

    void create_present_Semaphores() {
        VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        for (auto i = 0; i < maxFramesInFlight; i++) {
            VK_CHECK_RESULT(vkCreateSemaphore(handle_->get_device(), &semaphoreCI, nullptr, &presentSemaphores[i]));
        }
    }

    void create_renderSemaphores() {
        VkSemaphoreCreateInfo semaphoreCI{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        renderSemaphores.resize(handle_->get_swap_image_view().size());
        for (auto &semaphore: renderSemaphores) {
            VK_CHECK_RESULT(vkCreateSemaphore(handle_->get_device(), &semaphoreCI, nullptr, &semaphore));
        }
    }


    void destroy() {
        VK_CHECK_RESULT(vkDeviceWaitIdle(handle_->get_device()));
        for (auto i = 0; i < maxFramesInFlight; i++) {
            vkDestroyFence(handle_->get_device(), fences[i], nullptr);                //  这里还需要
            vkDestroySemaphore(handle_->get_device(), presentSemaphores[i], nullptr); //
            vmaUnmapMemory(handle_->get_allocator(), shaderDataBuffers[i].allocation);
            vmaDestroyBuffer(handle_->get_allocator(), shaderDataBuffers[i].buffer, shaderDataBuffers[i].allocation);
        }
        for (auto i = 0; i < renderSemaphores.size(); i++) {
            vkDestroySemaphore(handle_->get_device(), renderSemaphores[i], nullptr);
        }
    }
};


#endif //HELLO_MAC_ENGINE_H

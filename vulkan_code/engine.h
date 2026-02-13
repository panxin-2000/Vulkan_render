//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H
#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model[3];
    glm::vec4 lightPos{0.0f, -10.0f, 10.0f, 0.0f};
    uint32_t selected{1};
    uint32_t selected8{1};
    uint32_t selected7{1};
    uint32_t selected6{1};
};


struct uniform_buffer {
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceAddress deviceAddress{};
    void *mapped{nullptr};
};


class Engine {
    std::array<VkCommandBuffer, maxFramesInFlight> command_buffers_ = {};
    std::array<uniform_buffer, maxFramesInFlight> uniform_buffers;
    std::array<VkFence, maxFramesInFlight> fences_                 = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_ = {};
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
    void init() {
        create_command_buffer();
        create_shader_data_buffer();
        create_fences();
        create_present_Semaphores();
        create_renderSemaphores();

    }

    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences_;
    }

    VkFence &get_current_fences() {
        return get_fences()[frameIndex];
    }

    std::array<VkSemaphore, maxFramesInFlight> &get_presentSemaphores() {
        return present_semaphores_;
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


    std::array<VkCommandBuffer, maxFramesInFlight> &get_command_buffers() {
        return command_buffers_;
    }

    VkCommandBuffer &get_current_command_buffer() {
        return get_command_buffers()[frameIndex];
    }

    std::array<uniform_buffer, maxFramesInFlight> &get_shader_data_buffer() {
        return uniform_buffers;
    }

    uniform_buffer &get_current_shader_data_buffer() {
        return get_shader_data_buffer()[frameIndex];
    }


    const VkImage &get_current_swap_chain_image();

    const VkImageView &get_current_swap_image_view();

    void create_command_buffer();


    void create_shader_data_buffer();

    void create_fences();

    void create_present_Semaphores();

    void create_renderSemaphores();

    void put_one_image_to_screen();

    /**
     *
     * @param imageIndex 必须用 imageIndex 去找图像资源
     */
    void get_one_image_can_render();


    void destroy_and_recreate_fence_and_semaphore();

    void destroy();
};

void update_shader_data(Engine &engine);

#endif //HELLO_MAC_ENGINE_H

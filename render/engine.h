//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H
#include <list>

#include "vulkan_code/vulkan_utility.h"
#include <string>
#include <vector>

#include "vulkan_code/descriptor.h"
#include "vulkan_code/descriptor_pool.h"
#include <Eigen/Eigen>

#include "Descriptor_pool_manager.h"
#include "PBR_component.h"
#include "pbr_manager.h"
#include "vulkan_code/vulkan_buffer.h"
#include "vulkan_code/vulkan_image.h"
#include "vulkan_code/vulkan_backend.h"

#include "shader_resolve.h"


struct Engine {
private:
    // 也许这里需要一个 VK_backend 的指针
    std::vector<VkCommandPool> command_pools_                       = {};
    std::vector<VkSemaphore> render_to_image_semaphores_            = {};
    std::array<VkCommandBuffer, maxFramesInFlight> command_buffers_ = {};
    std::array<VkQueryPool, maxFramesInFlight> query_pools          = {};
    std::array<VkFence, maxFramesInFlight> fences_                  = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_  = {};

    std::vector<DescriptorSet_ptr> bindless_descriptor_sets_ = {};
    std::vector<DescriptorSet_ptr> global_descriptor_sets_   = {};


    std::vector<VKR_image_ptr> swap_chain_images_;
    std::vector<VKR_image_ptr> G_buffer_Position_images_;
    std::vector<VKR_image_ptr> g_buffer_Normal_images_;
    std::vector<VKR_image_ptr> G_buffer_BaseColor_images_;
    std::vector<VKR_image_ptr> depth_images_;

    VkSemaphore vk_timeline_semaphore_ = VK_NULL_HANDLE;
    std::atomic<uint64_t> framerate_   = 0;

    struct Global_parameters {
        Eigen::Matrix4f view_matrix;
        Eigen::Matrix4f projection_matrix;
        Eigen::Matrix4f inv_view_matrix;
        Eigen::Matrix4f inv_projection_matrix;
        Eigen::Matrix4f invVP;
        Eigen::Vector4f world_camera_pos;
        Eigen::Vector4f lightPos;
        Eigen::Vector4f screen_size;
    };


    Descriptor_pool_manager descriptor_pool_manager_;
    PBR_manager pbr_manager_;
    VKR_buffer_ptr pbr_components_buffer_;


    Global_parameters global_parameters_;

    shader_data gltf_shader_data_;

    std::shared_ptr<vk_shader_data> shader_date;
    std::map<std::string, Update_descriptor_binding> update_bindless_descriptor_sets_;

public:
    static Engine &instance();

    [[nodiscard]] uint64_t get_finished_timeline() const {
        uint64_t current_timeline;
        // todo: 偶尔出现一个这个错误，应该是两个线程之间的一个同步问题
        // 确定一下 这个 can't be called on VkImageView 出现后才会出现  assert 失败的情况
        // vkGetSemaphoreCounterValue(): semaphore Invalid VkSemaphore Object 0x0
        VkResult result = vkGetSemaphoreCounterValue(VK_backend::instance().get_device(), vk_timeline_semaphore_,
                                                     &current_timeline);
        assert(result == VK_SUCCESS && "vulkan get timeline semaphore value error");
        return current_timeline;
    }

    PBR_manager &get_pbr_manager() {
        return pbr_manager_;
    }

    void set_framerate(const uint64_t framerate) {
        framerate_ = framerate;
    }

    uint64_t get_framerate() {
        return framerate_;
    }


    void submit_render_queue(uint64_t time_line);

    void copy_image_to_screen();

    void get_image_to_render();

    void create_timeline_Semaphores();


    static uint64_t get_current_submit_timeline() {
        static std::atomic<uint64_t> time_line = 1;
        ++time_line;
        return time_line - 1; // 第一次拿到的时候就是 1
    }

    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

    uint32_t get_frameIndex() const {
        return frameIndex;
    }

    uint32_t get_imageIndex() const {
        return imageIndex;
    }

    bool set_projection_matrix(const Eigen::Matrix4f &matrix) {
        global_parameters_.projection_matrix = matrix;
        return true;
    }

    bool set_inv_projection_matrix(const Eigen::Matrix4f &matrix) {
        global_parameters_.inv_projection_matrix = matrix;
        return true;
    }

    bool set_view_matrix(const Eigen::Matrix4f &matrix) {
        global_parameters_.view_matrix = matrix;
        return true;
    }

    bool set_inv_view_matrix(const Eigen::Matrix4f &matrix) {
        global_parameters_.inv_view_matrix = matrix;
        return true;
    }

    bool set_invVP(const Eigen::Matrix4f &matrix) {
        global_parameters_.invVP = matrix;
        return true;
    }

    bool set_world_camera_pos(const Eigen::Vector3f &matrix) {
        global_parameters_.world_camera_pos = {matrix.x(), matrix.y(), matrix.z(), 0};
        return true;
    }

    bool set_world_light_pos(const Eigen::Vector3f &matrix) {
        global_parameters_.lightPos = {matrix.x(), matrix.y(), matrix.z(), 0};;
        return true;
    }

    bool set_screen_size(const Eigen::Vector2f &screen_size_t) {
        global_parameters_.screen_size = {screen_size_t.x(), screen_size_t.y(), 0, 0};;
        return true;
    }


    std::array<VkFence, maxFramesInFlight> &get_fences() {
        return fences_;
    }

    const VkCommandPool &get_command_pool(const uint index = 0) const {
        return command_pools_.at(index);
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

    VkQueryPool &get_current_query_pool() {
        return query_pools[frameIndex];
    }

    void get_query_results();

    void create_command_buffer();

    void destroy_command_buffer();

    void create_query_pool();

    void destroy_query_pool();

    void create_fences();

    void destroy_fences();

    void create_present_Semaphores();

    void destroy_present_Semaphores();

    void create_renderSemaphores();

    void destroy_renderSemaphores();

    void create_command_pool();

    void destroy_command_pool();


    std::vector<DescriptorSet_ptr> get_bindless_descriptor_set(const uint index = 0);

    std::vector<DescriptorSet_ptr> get_global_descriptor_set(const uint index = 0);

    void update_global_parameter();

    void update_global_pbr_parameter(std::map<std::string, Update_descriptor_binding> &update_global_descriptor_sets);

    void update_bindless_parameter();


    VkDescriptorPool get_descriptor_pool() const {
        return descriptor_pool_manager_.get_descriptor_pool_for_alloc();
    }

    void allocate_descriptor_pool() {
        descriptor_pool_manager_.allocate_descriptor_pool();
    }

    void create_render_image();

    [[nodiscard]] const VkImage &get_current_swap_chain_image(uint index = 0) const;

    [[nodiscard]] const VkImageView &get_current_swap_image_view(uint index = 0) const;

    [[nodiscard]] const VkImage &get_current_depth_image(uint index = 0) const;

    [[nodiscard]] const VkImageView &get_current_depth_view(uint index = 0) const;

    [[nodiscard]] const VKR_image_ptr &get_current_position_image_ptr(uint index = 0) const;

    [[nodiscard]] const VKR_image_ptr &get_current_normal_image_ptr(uint index = 0) const;

    [[nodiscard]] const VKR_image_ptr &get_current_baseColor_image_ptr(uint index = 0) const;

    [[nodiscard]] const VKR_image_ptr &get_current_depth_image_ptr(uint index = 0) const;

    [[nodiscard]] const VkImage &get_current_position_image(uint index = 0) const;

    [[nodiscard]] const VkImageView &get_current_position_view(uint index = 0) const;

    [[nodiscard]] const VkImage &get_current_normal_image(uint index = 0) const;

    [[nodiscard]] const VkImageView &get_current_normal_view(uint index = 0) const;

    [[nodiscard]] const VkImage &get_current_baseColor_image(uint index = 0) const;

    [[nodiscard]] const VkImageView &get_current_baseColor_view(uint index = 0) const;


    [[nodiscard]] const std::vector<VKR_image_ptr> &get_swap_chain_images() const {
        return swap_chain_images_;
    }

    [[nodiscard]] const std::vector<VKR_image_ptr> &get_depth_images() const {
        return depth_images_;
    }

    std::shared_ptr<vk_shader_data> get_gltf_shader_data() {
        return shader_date;
    }


    void destroy_render_image();

    void create();

    void recreate_swap_chain();

    void destroy();

    void destroy_and_recreate_fence_and_semaphore();

    void add_bindless_texture(const std::optional<Texture_parameter> &texture);

    void update_bindless_descriptor_sets_function();
};


#endif //HELLO_MAC_ENGINE_H

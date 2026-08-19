//
// Created by 潘鑫 on 2026/1/27.
//

#ifndef HELLO_MAC_ENGINE_H
#define HELLO_MAC_ENGINE_H
#include <list>
#include <oneapi/tbb.h>

#include "vulkan_code/vulkan_utility.h"
#include <string>
#include <vector>

#include "vulkan_code/descriptor.h"

#include "Descriptor_pool_manager.h"
#include "render_common/frustum.h"
#include "render_common/PBR_component.h"
#include "pbr_manager.h"
#include "render_image_manager.h"
#include "Shader_manager.h"
#include "vulkan_code/vulkan_buffer.h"
#include "vulkan_code/vulkan_image.h"
#include "vulkan_code/vulkan_backend.h"

#include "shader_resolve.h"
#include "vulkan_execute_command.h"

struct Global_parameters {
    Eigen::Matrix4f view_matrix;
    Eigen::Matrix4f projection_matrix;
    Eigen::Matrix4f inv_view_matrix;
    Eigen::Matrix4f inv_projection_matrix;
    Eigen::Matrix4f invVP;
    FrustumPlanes frustum_planes;
    Eigen::Vector4f world_camera_pos;
    Light light;
    Eigen::Vector4f screen_size;
    std::array<Eigen::Array4f, 9> shCoefficients;

    bool set_projection_matrix(const Eigen::Matrix4f &matrix) {
        projection_matrix = matrix;
        return true;
    }

    bool set_inv_projection_matrix(const Eigen::Matrix4f &matrix) {
        inv_projection_matrix = matrix;
        return true;
    }

    bool set_view_matrix(const Eigen::Matrix4f &matrix) {
        view_matrix = matrix;
        return true;
    }

    bool set_inv_view_matrix(const Eigen::Matrix4f &matrix) {
        inv_view_matrix = matrix;
        return true;
    }

    bool set_invVP(const Eigen::Matrix4f &matrix) {
        invVP = matrix;
        return true;
    }

    bool set_world_camera_pos(const Eigen::Vector3f &v3) {
        world_camera_pos = {v3.x(), v3.y(), v3.z(), 0};
        return true;
    }

    bool set_sun_light(const Eigen::Vector3f &v3) {
        light.set_color(1.0f, 0.98f, 0.95f);
        light.set_intensity(5.0f);
        auto tem = v3;
        tem.normalize();
        light.set_rotate({tem.x(), tem.y(), tem.z(), 0.0f});
        return true;
    }

    bool set_screen_size(const Eigen::Vector2f &screen_size_t) {
        screen_size = {screen_size_t.x(), screen_size_t.y(), 0, 0};;
        return true;
    }
};


struct Engine {
private:
    // 也许这里需要一个 VK_backend 的指针
    std::vector<VkCommandPool> command_pools_                       = {};
    std::vector<VkSemaphore> render_to_image_semaphores_            = {};
    std::array<VkCommandBuffer, maxFramesInFlight> command_buffers_ = {};
    std::array<VkQueryPool, maxFramesInFlight> query_pools          = {};
    std::array<VkFence, maxFramesInFlight> fences_                  = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_  = {};

    std::vector<DescriptorSet_ptr> bindless_descriptor_sets_              = {};
    std::array<std::vector<DescriptorSet_ptr>, 3> global_descriptor_sets_ = {};
    uint32_t global_descriptor_sets_index                                 = 0;

    std::vector<VKR_image_ptr> swap_chain_images_;

    Render_image_manager render_image_manager_;
    Descriptor_pool_manager descriptor_pool_manager_;
    Command_submit_manager command_submit_manager_;
    Shader_manager shader_manager_;
    PBR_manager pbr_manager_;

    VkSemaphore vk_timeline_semaphore_ = VK_NULL_HANDLE;
    std::atomic<uint64_t> framerate_   = 0;

    VKR_buffer_ptr pbr_components_buffer_;

    Global_parameters global_parameters_;

    std::map<std::string, Update_descriptor_binding> update_bindless_descriptor_sets_;

    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

public:
    static Engine &instance();

    [[nodiscard]] uint64_t get_finished_timeline() const;

    PBR_manager &get_pbr_manager() {
        return pbr_manager_;
    }

    Global_parameters &get_global_parameters() {
        return global_parameters_;
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


    uint32_t get_frameIndex() const {
        return frameIndex;
    }

    uint32_t get_imageIndex() const {
        return imageIndex;
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

    void update_global_parameter(std::optional<Texture_parameter> offscreen,
                                 std::optional<Texture_parameter> SSAO,
                                 std::optional<Texture_parameter> depth);

    FrustumPlanes get_frustum_planes() const {
        return global_parameters_.frustum_planes;
    }

    Eigen::Vector4f get_world_camera_pos() const {
        return global_parameters_.world_camera_pos;
    }

    void update_global_pbr_parameter(std::map<std::string, Update_descriptor_binding> &update_global_descriptor_sets);

    void update_bindless_parameter();

    VkDescriptorPool get_descriptor_pool() const {
        return descriptor_pool_manager_.get_descriptor_pool_for_alloc();
    }

    void allocate_descriptor_pool() {
        descriptor_pool_manager_.allocate_descriptor_pool();
    }

    void create_render_image();

    [[nodiscard]] const VKR_image_ptr &get_current_swap_chain_image() const;

    Render_image_manager &get_image_manager() {
        return render_image_manager_;
    }

    [[nodiscard]] const std::vector<VKR_image_ptr> &get_swap_chain_images() const {
        return swap_chain_images_;
    }

    Shader_manager get_shader_manager() {
        return shader_manager_;
    }

    Command_submit_manager &get_command_submit_manager() {
        return command_submit_manager_;
    }

    void shader_manager_destroy() {
        descriptor_pool_manager_.clean_shader_data();
        shader_manager_.destroy();
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

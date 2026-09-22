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
#include "Geometry_data.h"
#include "global_parameters.h"
#include "render_common/frustum.h"
#include "render_common/PBR_component.h"
#include "pbr_manager.h"
#include "render_image_manager.h"
#include "render_mesh.h"
#include "Shader_manager.h"
#include "vulkan_code/vulkan_buffer.h"
#include "vulkan_code/vulkan_image.h"

#include "shader_resolve.h"
#include "vulkan_execute_command.h"
#include "utils/StructureOfArrays.h"


struct Engine {
private:
    // 也许这里需要一个 VK_backend 的指针
    std::vector<VkCommandPool> command_pools_                = {};
    std::vector<DescriptorSet_ptr> bindless_descriptor_sets_ = {};

    std::array<VkCommandBuffer, 3> command_buffers_                       = {};
    std::array<VkQueryPool, maxFramesInFlight> query_pools                = {};
    std::array<VKR_buffer_ptr, maxFramesInFlight> pickup_buffers_         = {};
    std::array<std::vector<DescriptorSet_ptr>, 3> global_descriptor_sets_ = {};
    uint32_t global_descriptor_sets_index                                 = 0;


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
    std::map<std::string, Update_descriptor_binding> update_bindless_sampler_sets_;

    uint32_t current_CSM        = 0;
    bool update_frustum_culling = true;


    VKR_buffer_ptr SOA_buffer_;

    struct Static_Model {
    };

public:
    using RenderableSoa = utils::StructureOfArrays<
        VKR_Primitive,    //
        Local_Space_AABB, //
        World_Space_AABB, //
        Transform_Matrix, //
        uint32_t,         // material_index 这里是索引,那么就应该去拿全局的 material
        entt::entity,     //
        VKR_Primitive,    //
        VKR_Primitive     //
    >;

    // instance 就变成了  SOA 的起始位置,   Transform 就需要去拿 entt 的 偏移了
    enum {
        SOA_Primitive,              //   4 | instance of the Renderable component
        SOA_Local_Space_AABB,       //   4 | instance of the Renderable component
        SOA_World_Space_AABB,       //   4 | instance of the Renderable component
        SOA_Transform_Matrix,       //   4 | instance of the Renderable component
        SOA_material_index,         //   4 | instance of the Renderable component
        SOA_entity,                 //   4 | instance of the Renderable component
        SOA_frustum_culling,        //   4 | instance of the Renderable component
        SOA_shadow_frustum_culling, //   4 | instance of the Renderable component
    };

private:
    RenderableSoa mRenderableData_;
    Mesh_data mesh_data_;
    VKR_buffer_ptr CommandCountBuffer_;
    Geometry_data bindless_Geometry_data_;

public:
    static Engine &instance();

    const RenderableSoa &get_renderable_SOA_data() const {
        return mRenderableData_;
    }

    const Mesh_data &get_mesh_data() const {
        return mesh_data_;
    }

    const VKR_buffer_ptr &get_SOA_buffer() const {
        return SOA_buffer_;
    }

    const VKR_buffer_ptr &get_CommandCountBuffer_() const {
        return CommandCountBuffer_;
    }


    uint32_t &get_current_CSM() {
        return current_CSM;
    }

    bool &get_frustum_culling() {
        return update_frustum_culling;
    }

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


    void create_timeline_Semaphores();

    static uint64_t get_current_submit_timeline() {
        static std::atomic<uint64_t> time_line = 1;
        ++time_line;
        return time_line - 1; // 第一次拿到的时候就是 1
    }


    const VkCommandPool &get_command_pool(const uint index = 0) const {
        return command_pools_.at(index);
    }


    VkSemaphore &get_timeline_semaphore() {
        return vk_timeline_semaphore_;
    }


    VkCommandBuffer &get_current_command_buffer() {
        return command_buffers_[global_descriptor_sets_index];
    }

    VkQueryPool &get_current_query_pool() {
        return query_pools[0];
    }

    void get_query_results();

    void create_command_buffer();

    void destroy_command_buffer();

    void create_query_pool();

    void destroy_query_pool();


    void create_command_pool();

    void destroy_command_pool();

    std::vector<DescriptorSet_ptr> get_bindless_descriptor_set(const uint index = 0);

    std::vector<DescriptorSet_ptr> get_global_descriptor_set(const uint index = 0);

    void update_global_parameter(std::optional<Texture_parameter> offscreen,
                                 std::optional<Texture_parameter> depth,
                                 std::optional<Texture_parameter> blur_SSAO,
                                 std::optional<Texture_parameter> shadow_texture);

    FrustumPlanes get_frustum_planes() const {
        return global_parameters_.frustum_planes;
    }

    std::array<FrustumPlanes, 4> get_light_frustum_planes() const {
        return global_parameters_.light_frustum_planes;
    }


    Eigen::Vector4f get_world_camera_pos() const {
        return global_parameters_.world_camera_pos;
    }

    void update_global_pbr_parameter(std::map<std::string, Update_descriptor_binding> &update_global_descriptor_sets);

    void update_SOA();

    void update_bindless_parameter();

    VkDescriptorPool get_descriptor_pool() const {
        return descriptor_pool_manager_.get_descriptor_pool_for_alloc();
    }

    void allocate_descriptor_pool() {
        descriptor_pool_manager_.allocate_descriptor_pool();
    }


    Render_image_manager &get_image_manager() {
        return render_image_manager_;
    }


    [[nodiscard]] const VKR_buffer_ptr &get_pickup_buffers() const {
        return pickup_buffers_[0];
    }

    Shader_manager &get_shader_manager() {
        return shader_manager_;
    }

    Command_submit_manager &get_command_submit_manager() {
        return command_submit_manager_;
    }

    void shader_manager_destroy() {
        descriptor_pool_manager_.clean_shader_data();
        shader_manager_.destroy();
    }


    void create();


    void destroy();


    void add_bindless_texture(const std::optional<Texture_parameter> &texture);

    void add_bindless_sampler(const std::optional<Texture_parameter> &texture);

    void update_bindless_descriptor_sets_function();
};


#endif //HELLO_MAC_ENGINE_H

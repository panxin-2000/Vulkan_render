//
// Created by 潘鑫 on 2026/2/14.
//
#include "engine.h"

#include <sys/stat.h>

#include "load_gltf_model.h"
#include "render_mesh.h"
#include "vulkan_code/sets_and_bindings_layout.h"
#include "shader_component.h"
#include "transform_component.h"
#include "utils/StructureOfArrays.h"
#include "vulkan_code/vulkan_backend.h"
#include "vulkan_code/vulkan_sample.h"


static std::atomic<Engine *> engine_instance{nullptr};

Engine &Engine::instance() {
    // 1. 第一次读取（使用 Acquire 保证能看到初始化后的完整内存）
    Engine *current = engine_instance.load(std::memory_order_acquire);
    if (current == nullptr) {
        // 2. 抢输了的线程，或者刚进来的线程，都在这里准备
        const auto new_value = new Engine();

        // 【关键修复】：在把指针暴露给全局之前，在线程私有空间内彻底把句柄初始化好！
        Engine *expected = nullptr;
        // 3. 经典的无锁自旋尝试
        // 如果 instance 是 expected(nullptr)，就写入 new_value
        if (engine_instance.compare_exchange_strong(expected, new_value,
                                                    std::memory_order_release,
                                                    std::memory_order_acquire)) {
            // 抢赢了！
            current = new_value;
        } else {
            // 抢输了！说明别的线程已经把一个【完全初始化好】的单例塞进 instance 了
            // new_value();
            delete new_value;   // 销毁自己这个备胎
            current = expected; // expected 已经被 CAS 自动更新为抢赢线程的那个完整指针
        }
    }
    return *current;
}

uint64_t Engine::get_finished_timeline() const {
    uint64_t current_timeline;
    // todo: 偶尔出现一个这个错误，应该是两个线程之间的一个同步问题
    // 确定一下 这个 can't be called on VkImageView 出现后才会出现  assert 失败的情况
    // vkGetSemaphoreCounterValue(): semaphore Invalid VkSemaphore Object 0x0
    VkResult result = vkGetSemaphoreCounterValue(VK_backend::instance().get_device(), vk_timeline_semaphore_,
                                                 &current_timeline);
    assert(result == VK_SUCCESS && "vulkan get timeline semaphore value error");
    return current_timeline;
}


void Engine::get_query_results() {
    const auto &backend = VK_backend::instance();
    if (get_current_query_pool() != VK_NULL_HANDLE) {
        uint64_t timestamps[2]; // 准备接收数组
        VkResult result = vkGetQueryPoolResults(
                                                backend.get_device(),
                                                get_current_query_pool(),
                                                0,                  // 从 index 0 开始
                                                2,                  // 获取 2 个结果
                                                sizeof(timestamps), // 总大小 16 字节
                                                timestamps,         // 目标数组
                                                sizeof(uint64_t),   // 每个元素的步长
                                                VK_QUERY_RESULT_64_BIT
                                               );
        if (result == VK_SUCCESS) {
            uint64_t start = timestamps[0];
            uint64_t end   = timestamps[1];
            // 计算耗时 (ns) = (end - start) * timestampPeriod
        } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        } else if (result == VK_NOT_READY) {
        }
    }
}

void Engine::create_query_pool() {
    const auto &backend = VK_backend::instance();
    VkQueryPoolCreateInfo queryPoolInfo{};
    queryPoolInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP; // 指定为时间戳类型
    queryPoolInfo.queryCount = 2;                       // 比如：一个存起点，一个存终点
    for (auto i = 0; i < maxFramesInFlight; i++) {
        if (vkCreateQueryPool(backend.get_device(), &queryPoolInfo, nullptr, &query_pools[i]) != VK_SUCCESS) {
            LOG_INFO(g_log(), "vkCreateQueryPool failed");
        } else {
            // vkResetQueryPool(handle.device_, queryPool, 0, 2);
            // 需要 VK_EXT_host_query_reset 的扩展
        }
    }
}

void Engine::destroy_query_pool() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < command_buffers_.size(); i++) {
        vkDestroyQueryPool(backend.get_device(), query_pools[i], nullptr);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create_command_buffer() {
    const auto &backend = VK_backend::instance();

    VkCommandBufferAllocateInfo cbAllocCI{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = get_command_pool(),
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(command_buffers_.size()),
    };
    VK_CHECK_RESULT_NOT_EXIT(vkAllocateCommandBuffers(backend.get_device(), &cbAllocCI,
                                 command_buffers_.data()));
}

void Engine::destroy_command_buffer() {
    const auto &backend = VK_backend::instance();
    for (auto i = 0; i < maxFramesInFlight; i++) {
        vkFreeCommandBuffers(backend.get_device(), get_command_pool(), 1, &command_buffers_[i]);
        command_buffers_[i] = VK_NULL_HANDLE;
    }
}


void Engine::create() {
    // pickup_buffers_
    for (auto i = 0; i < maxFramesInFlight; i++) {
        pickup_buffers_[i] = create_vma_buffer(ALIGN_256(11 * 11 * 4), VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                                               VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                               VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT);
    }

    descriptor_pool_manager_.create();
    command_submit_manager_.create();
    shader_manager_.create();
    auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
    descriptor_pool_manager_.set_shader_data(gltf_shader_data);
    bindless_descriptor_sets_ =
            descriptor_pool_manager_.allocate_bindless_descriptor_sets(
                                                                       gltf_shader_data->bindless_sets_bindings,
                                                                       gltf_shader_data->bindless_set_layout);
    global_descriptor_sets_[0] = descriptor_pool_manager_.allocate_global_descriptor_sets(
         gltf_shader_data->global_sets_bindings,
         gltf_shader_data->global_descriptor_sets_layout
        );
    global_descriptor_sets_[1] = descriptor_pool_manager_.allocate_global_descriptor_sets(
         gltf_shader_data->global_sets_bindings,
         gltf_shader_data->global_descriptor_sets_layout
        );
    global_descriptor_sets_[2] = descriptor_pool_manager_.allocate_global_descriptor_sets(
         gltf_shader_data->global_sets_bindings,
         gltf_shader_data->global_descriptor_sets_layout
        );

    // 需要在这里创建一些内容
    // 或者说，到这里之后才能够进行上传
    {
        // 添加一张纯白的背景图片
        auto texture_default_color_ = create_single_color_texture(0x7f, 0x7f, 0x7f);
        add_bindless_texture(texture_default_color_);
        add_bindless_sampler(texture_default_color_);
        // 默认 指向于 z 轴的 法线
        auto texture_default_normal_ = create_single_color_texture(128, 128, 255);
        add_bindless_texture(texture_default_normal_);
        PBR_Texture_ptr ptr = {
            texture_default_color_,
            texture_default_normal_,
            {},
            {}
        };
        pbr_manager_.push({}, ptr);
    }
    // {
    //     // std::optional<Texture_parameter> texture = create_texture_from_image( );
    //     // uint32_t index                           = add_bindless_uniform_sampler2D("default_text_MSDF_texture", texture);
    //     // assert(index == 1);
    // }


    create_command_pool();
    create_command_buffer();
    render_image_manager_.create();
    create_timeline_Semaphores();
}


void Engine::add_bindless_texture(const std::optional<Texture_parameter> &texture) {
    auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
    for (auto const &[set_value, bindings_map]: gltf_shader_data->bindless_sets_bindings) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == "bindless_texture2D") {
                uint32_t index = texture.value().image->get_index();
                Update_descriptor_binding temp;
                temp.binding_name                                       = "bindless_texture2D";
                temp.resource_type                                      = "uniform sampler2D";
                temp.dstSet                                             = set_value;
                temp.descriptor_write_binding.sType                     = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                temp.descriptor_write_binding.dstBinding                = binding_value;
                temp.descriptor_write_binding.dstArrayElement           = index;
                temp.descriptor_write_binding.descriptorCount           = 1;
                temp.descriptor_write_binding.pBufferInfo               = nullptr;
                temp.descriptor_write_binding.pImageInfo                = nullptr;
                temp.descriptor_write_binding.pTexelBufferView          = nullptr;
                temp.descriptor_write_binding.descriptorType            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                temp.texture_info                                       = {true, texture.value()};
                update_bindless_descriptor_sets_[std::to_string(index)] = temp;
                // 多线程的情况下 偶现 出现问题
            }
        }
    }
}

void Engine::add_bindless_sampler(const std::optional<Texture_parameter> &texture) {
    auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
    for (auto const &[set_value, bindings_map]: gltf_shader_data->bindless_sets_bindings) {
        for (const auto &[binding_value, info]: bindings_map) {
            if (info.binding_name == "bindless_samplers") {
                uint32_t index = texture.value().sampler.get_index();
                Update_descriptor_binding temp;
                temp.binding_name                                    = "bindless_samplers";
                temp.resource_type                                   = "uniform sampler2D";
                temp.dstSet                                          = set_value;
                temp.descriptor_write_binding.sType                  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                temp.descriptor_write_binding.dstBinding             = binding_value;
                temp.descriptor_write_binding.dstArrayElement        = index;
                temp.descriptor_write_binding.descriptorCount        = 1;
                temp.descriptor_write_binding.pBufferInfo            = nullptr;
                temp.descriptor_write_binding.pImageInfo             = nullptr;
                temp.descriptor_write_binding.pTexelBufferView       = nullptr;
                temp.descriptor_write_binding.descriptorType         = VK_DESCRIPTOR_TYPE_SAMPLER;
                temp.texture_info                                    = {true, texture.value()};
                update_bindless_sampler_sets_[std::to_string(index)] = temp; // 这一行 还是有问题的
            }
        }
    }
}

void Engine::update_bindless_descriptor_sets_function() {
    update_descriptor_sets(update_bindless_descriptor_sets_, bindless_descriptor_sets_);
    update_descriptor_sets(update_bindless_sampler_sets_, bindless_descriptor_sets_);
    update_bindless_descriptor_sets_.clear();
    update_bindless_sampler_sets_.clear();
}


void Engine::destroy() {
    frustum_culling_buffer_.clear();
    world_space_aabb_buffer_.clear();
    vkr_primitive_buffer_.clear();
    shadow_frustum_culling_buffer_.clear();
    transform_matrix_buffer_.clear();
    uint32_t_buffer_.clear();
    entity_buffer_.clear();


    shader_manager_destroy();

    for (auto i = 0; i < maxFramesInFlight; i++) {
        pickup_buffers_[i] = nullptr;
    }


    pbr_manager_.destroy();
    command_submit_manager_.destroy();
    if (pbr_components_buffer_ != nullptr) {
        pbr_components_buffer_ = {};
    }
    update_bindless_descriptor_sets_.clear();
    const auto &backend = VK_backend::instance();
    VK_CHECK_RESULT_NOT_EXIT(vkDeviceWaitIdle(backend.get_device()));

    // 这里的顺序不对
    render_image_manager_.destroy();

    // 需要强制清除
    constexpr uint64_t finished_timeline = std::numeric_limits<uint64_t>::max();
    discard_descriptor_set_map_clean(finished_timeline);
    discard_buffer_map_clean(finished_timeline);
    discard_image_and_view_map_clean(finished_timeline);

    // 销毁 timeline_semaphore 再全部检查一遍再销毁
    destroy_all_vulkan_sample();


    vkDestroySemaphore(VK_backend::instance().get_device(), vk_timeline_semaphore_, nullptr);
    vk_timeline_semaphore_ = VK_NULL_HANDLE;


    destroy_command_buffer();
    destroy_command_pool();
    descriptor_pool_manager_.destroy();
}

void Engine::destroy_command_pool() {
    const auto &backend = VK_backend::instance();
    for (auto command_pool: command_pools_) {
        if (command_pool != VK_NULL_HANDLE)
            vkDestroyCommandPool(backend.get_device(), command_pool, nullptr);
    }
}

void Engine::create_command_pool() {
    // Command pool
    const auto &backend = VK_backend::instance();
    const VkCommandPoolCreateInfo commandPoolCI{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = backend.get_queue_Family()
    };
    VkCommandPool commandPool;
    VK_CHECK_RESULT(vkCreateCommandPool(backend.get_device(), &commandPoolCI, nullptr, &commandPool));
    command_pools_.push_back(commandPool);
}


void Engine::update_global_pbr_parameter(
    std::map<std::string, Update_descriptor_binding> &update_global_descriptor_sets) {
    auto size = pbr_manager_.size() * sizeof(PBR_component);
    if (size > 0) {
        auto src = pbr_manager_.data();

        if (pbr_components_buffer_ == nullptr) {
            pbr_components_buffer_ = create_SSBO_buffer(ALIGN_1024(size * 2));
        }
        if (pbr_components_buffer_.get()->get_size() < size) {
            pbr_components_buffer_ = create_SSBO_buffer(ALIGN_1024(size * 2));
        }
        auto mem_copy_function = [src,size](void *dst) {
            memcpy(dst, src, size);
        };


        copy_mem_from_cpu_to_gpu(pbr_components_buffer_, mem_copy_function);
        auto gltf_shader_data = shader_manager_.get_gltf_shader_data();
        VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                            "global_PBR_parameters", pbr_components_buffer_);
    }
}


void Engine::update_SOA() {
    auto view = Render_entt().view<std::vector<VKR_Primitive>,
                                   std::vector<World_Space_AABB>,
                                   std::vector<Transform_Matrix>,
                                   Gltf_material_parameters,
                                   screen_pick_entity>();
    // 需要做什么呢? // entity 的 作为 ,
    // 之后需要做什么呢? 计算
}


void Engine::update_global_parameter(std::optional<Texture_parameter> offscreen,
                                     std::optional<Texture_parameter> depth,
                                     std::optional<Texture_parameter> blur_SSAO,
                                     std::optional<Texture_parameter> shadow_texture) {
    auto gltf_shader_data        = shader_manager_.get_gltf_shader_data();
    global_descriptor_sets_index = (global_descriptor_sets_index + 1) % 3;
    const auto extent            = VK_backend::instance().get_current_extent();
    std::map<std::string, Update_descriptor_binding> update_global_descriptor_sets;

    global_parameters_.screen_size = {
        static_cast<float>(extent.width), static_cast<float>(extent.height), 0, 0
    };

    global_parameters_.frustum_planes =
            get_Frustum_Planes(global_parameters_.projection_matrix * global_parameters_.view_matrix);

    std::array<Eigen::Array4f, 9> &shCoefficients = global_parameters_.shCoefficients;
    // 值是一个差不多结果,不是很精准,因为 输出的 时候只保存了两位小数
    shCoefficients[0] = {1.73, 1.73, 1.73, 0.0f};
    shCoefficients[1] = {-0.05, -0.05, -0.05, 0.0f};
    shCoefficients[2] = {-0.16, -0.16, -0.16, 0.0f};
    shCoefficients[3] = {0.01, 0.01, 0.01, 0.0f};
    shCoefficients[4] = {-0.0, -0.0, -0.0, 0.0f};
    shCoefficients[5] = {0.02, 0.02, 0.02, 0.0f};
    shCoefficients[6] = {0.03, 0.03, 0.03, 0.0f};
    shCoefficients[7] = {-0.01, -0.01, -0.01, 0.0f};
    shCoefficients[8] = {0.01, 0.01, 0.01, 0.0f};

    shCoefficients[0] *= 0.282095f;
    // 对应 l=1
    shCoefficients[1] *= -0.488603f;
    shCoefficients[2] *= 0.488603f;
    shCoefficients[3] *= -0.488603f;

    // 对应 l=2
    shCoefficients[4] *= 1.092548f;
    shCoefficients[5] *= -1.092548f;
    shCoefficients[6] *= 0.315392f;
    shCoefficients[7] *= -1.092548f;
    shCoefficients[8] *= 0.546274f;


    global_parameters_.update_directional_light(get_current_CSM());
    global_parameters_.set_render_timeline(0);
    VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                        "global_parameters", global_parameters_);
    VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                        "global_offscreen", offscreen);
    VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                        "global_depth", depth);
    VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                        "global_Blur_SSAO", blur_SSAO);
    VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                        "global_shadow_texture", shadow_texture);

    auto view = Render_entt().view<std::vector<VKR_Primitive>,
                                   std::vector<World_Space_AABB>,
                                   std::vector<Transform_Matrix>,
                                   Gltf_material_parameters,
                                   screen_pick_entity>();

    size_t primitive_size = 0;
    for (const auto entity: view) {
        auto size      = Render_entt().get<std::vector<VKR_Primitive> >(entity).size();
        primitive_size += size;
    }

    if (primitive_size > 0) {
        mRenderableData_.clear();
        mRenderableData_.setCapacity(primitive_size);
        // soa.push_back(0.0f, 1.0, destroyedFloat4);
        for (const auto entity: view) {
            auto size = Render_entt().get<std::vector<VKR_Primitive> >(entity).size();
            for (uint i = 0; i < size; ++i) {
                auto soa_size = mRenderableData_.size();
                mRenderableData_.push_back();
                mRenderableData_.elementAt<SOA_Primitive>(soa_size) =
                        Render_entt().get<std::vector<VKR_Primitive> >(entity).at(i);
                mRenderableData_.elementAt<SOA_World_Space_AABB>(soa_size) =
                        Render_entt().get<std::vector<World_Space_AABB> >(entity).at(i);
                mRenderableData_.elementAt<SOA_Transform_Matrix>(soa_size) =
                        Render_entt().get<std::vector<Transform_Matrix> >(entity).at(i);
                mRenderableData_.elementAt<SOA_material_index>(soa_size) =
                        Render_entt().get<Gltf_material_parameters>(entity).at(i);
                mRenderableData_.elementAt<SOA_entity>(soa_size) =
                        Render_entt().get<screen_pick_entity>(entity).at(i);
            }
        }

        auto buffer_size      = RenderableSoa::getNeededSize(mRenderableData_.capacity());
        auto SOA_buffer       = create_SSBO_buffer(buffer_size, mRenderableData_.data<0>());
        vkr_primitive_buffer_ = VKR_buffer_offset{
            SOA_buffer, mRenderableData_.capacity() * sizeof(VKR_Primitive),
            RenderableSoa::getOffset(0, mRenderableData_.capacity())
        };
        world_space_aabb_buffer_ = VKR_buffer_offset{
            SOA_buffer, mRenderableData_.capacity() * sizeof(World_Space_AABB),
            RenderableSoa::getOffset(1, mRenderableData_.capacity())
        };
        transform_matrix_buffer_ = VKR_buffer_offset{
            SOA_buffer, mRenderableData_.capacity() * sizeof(Transform_Matrix),
            RenderableSoa::getOffset(2, mRenderableData_.capacity())
        };
        uint32_t_buffer_ = VKR_buffer_offset{
            SOA_buffer, mRenderableData_.capacity() * sizeof(uint32_t),
            RenderableSoa::getOffset(3, mRenderableData_.capacity())
        };
        entity_buffer_ = VKR_buffer_offset{
            SOA_buffer, mRenderableData_.capacity() * sizeof(entt::entity),
            RenderableSoa::getOffset(4, mRenderableData_.capacity())
        };
        // 这里才是正确的内容 ,之后呢 ? 想办法
        VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                            "global_model_matrix", transform_matrix_buffer_);
        VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                            "global_entity_to_screen", entity_buffer_);
        VK_update_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
                            "global_material_parameters", uint32_t_buffer_);

        frustum_culling_buffer_ = VKR_buffer_offset{
            create_SSBO_buffer(sizeof(VKR_Primitive) * mRenderableData_.capacity(),
                               mRenderableData_.data<0>())
        };
        shadow_frustum_culling_buffer_ = VKR_buffer_offset{
            create_SSBO_buffer(sizeof(VKR_Primitive) * mRenderableData_.capacity(),
                               mRenderableData_.data<0>())
        };
    }


    // set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
    //                      "global_compute_Texture", compute_texture);
    //
    // set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
    //                      "global_SSAO_compute_write", SSAO);
    // set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
    //                      "global_Blur_SSAO_compute_write", blur_write_SSAO);
    // set_render_parameter(gltf_shader_data->global_sets_bindings, update_global_descriptor_sets,
    //                      "global_dof_BlurImg", dof_blur);

    update_global_pbr_parameter(update_global_descriptor_sets);
    Proxy_descriptor_sets descriptor_sets; // 这里是需要按照顺序的
    auto bindless_descriptor_sets = get_bindless_descriptor_set();
    auto global_descriptor_sets   = get_global_descriptor_set();
    // 先使用下面的直接引用，之后再看怎么获取父节点的全局索引
    // auto &global_descriptor_sets = vk_s_d_s->global_descriptor_sets;
    descriptor_sets.reserve(bindless_descriptor_sets.size() +
                            global_descriptor_sets.size());
    descriptor_sets.insert(descriptor_sets.end(),
                           bindless_descriptor_sets.begin(),
                           bindless_descriptor_sets.end());
    descriptor_sets.insert(descriptor_sets.end(),
                           global_descriptor_sets.begin(),
                           global_descriptor_sets.end());
    update_descriptor_sets(update_global_descriptor_sets, descriptor_sets);
}

void Engine::update_bindless_parameter() {
    update_descriptor_sets(update_bindless_descriptor_sets_, bindless_descriptor_sets_);
    // 看起来确实很简单，只有这一个函数
    // 某些内容写过一次之后是不需要去重新再去写的
}


Proxy_descriptor_sets Engine::get_bindless_descriptor_set(const uint index) {
    return bindless_descriptor_sets_;
}

Proxy_descriptor_sets Engine::get_global_descriptor_set(const uint index) {
    return global_descriptor_sets_[global_descriptor_sets_index];
}


void Engine::create_timeline_Semaphores() {
    VkSemaphoreTypeCreateInfo vk_semaphore_type_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, nullptr, VK_SEMAPHORE_TYPE_TIMELINE, 0
    };
    VkSemaphoreCreateInfo vk_semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, &vk_semaphore_type_create_info, 0
    };
    vkCreateSemaphore(VK_backend::instance().get_device(), &vk_semaphore_create_info, nullptr, &vk_timeline_semaphore_);
}

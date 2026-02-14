//
// Created by 潘鑫 on 2026/1/22.
//

#ifndef HELLO_MAC_GLFW_VULKAN_H
#define HELLO_MAC_GLFW_VULKAN_H

#include "vulkan_utility.h"
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "engine.h"
#include "global_singleton.h"
#include "logic_render_data.h"


struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct Texture_parameter {
    VmaAllocation allocation  = VK_NULL_HANDLE;
    VkImage image             = VK_NULL_HANDLE;
    VkImageView image_view    = VK_NULL_HANDLE;
    VkSampler sampler         = VK_NULL_HANDLE;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

class VK_handle {
public:
    // ApplicationInfo 的参数
    std::string application_name_ = "Vulkan Example";
    std::string engine_name_      = "no engine";
    uint32_t application_version_ = 102;
    uint32_t engine_version_      = 0;
    uint32_t api_version_         = VK_API_VERSION_1_3;


    std::vector<const char *> instanceExtensions;


    // 需要给外部看到的变量
    GLFWwindow *window_               = nullptr;
    VkInstance instance_              = VK_NULL_HANDLE;
    VkSurfaceKHR surface_             = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_                  = VK_NULL_HANDLE;
    VkQueue graphics_queue_           = VK_NULL_HANDLE;
    VkQueue present_queue_            = VK_NULL_HANDLE;
    VkQueue transfer_queue_           = VK_NULL_HANDLE;
    VkQueue compute_queue_            = VK_NULL_HANDLE;

    // Engine engine_;
    std::array<VkCommandBuffer, maxFramesInFlight> command_buffers_ = {};
    std::array<uniform_buffer, maxFramesInFlight> uniform_buffers_  = {};
    std::array<VkFence, maxFramesInFlight> fences_                  = {};
    std::array<VkSemaphore, maxFramesInFlight> present_semaphores_  = {};
    std::vector<VkSemaphore> render_to_image_semaphores_;
    /**
     * frameIndex 正在渲染的一帧图像
     * imageIndex swap chain 创建的 image 的索引
     * 可能会有三个 image 交替显示到屏幕
     * 但是永远只有 一个 image 用于渲染
     * 屏幕绘制比较快的话，三缓冲没有太大的作用
    * 屏幕绘制比较慢的话，丢弃过时帧，选择最新帧绘制，开始渲染到开始显示的延迟的延迟不一致的问题
     */
    uint32_t frameIndex{0};
    uint32_t imageIndex{0};
    // Engine engine_;
public:
    void engine_init() {
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
        return uniform_buffers_;
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

    void engine_destroy();


    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    std::vector<VkImageView> swap_chain_image_views_;

    // 为什么会多一个这个？   内存屏障的时候需要用到，清理的时候不用清理，由swap chain 清理
    std::vector<VkImage> swap_chain_images_;

    VkImage depth_image_{};
    VkImageView depth_image_view_{};

    VmaAllocator allocator_ = VK_NULL_HANDLE; // 之后需要添加的另一个项目中
    uint32_t queue_family_{0};                // 不清楚是否能够删除
    VkFormat depth_format_{VK_FORMAT_UNDEFINED};

    struct {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    } queueFamilyIndices{}; // 不用给到外部，

    bool framebufferResized = false;

    uint32_t descriptor_count_ = 500;                //static_cast<uint32_t>(textures.size())
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE}; // 最大的问题就是这里有一个pool


    auto &get_indices_map() {
        return indices_map_;
    }

    auto &get_pipeline_map() {
        return pipeline_map_;
    }

    auto &get_mesh_map() {
        return mesh_map_;
    }

    auto &get_texture_map() {
        return texture_map_;
    }

    auto &get_shader_map() {
        return shader_maps_;
    }

    auto &get_pipeline_layout_map() {
        return pipeline_layout_map_;
    }

    auto &get_descriptor_sets_layout_map() {
        return descriptor_sets_layout_map_;
    }

    const VkCommandPool &get_command_pool() const {
        return commandPool;
    }


    VkDescriptorPool get_descriptor_pool() {
        return descriptorPool;
    }

    auto &get_bindless_textures() {
        return bindless_textures_;
    }

private:
    std::map<std::string, shader_and_share> shader_maps_;
    std::map<std::string, texture_and_share> texture_map_;
    std::map<std::string, std::pair<std::vector<VkDescriptorSetLayout>, uint32_t> > descriptor_sets_layout_map_;
    std::map<std::string, std::pair<VkPipelineLayout, uint32_t> > pipeline_layout_map_;
    std::map<std::string, pipeline_and_share> pipeline_map_;
    std::map<logic_render_data *, buffer_and_share> mesh_map_;

    std::map<Indices_type, buffer_and_share> indices_map_;
    std::vector<VkDescriptorImageInfo> bindless_textures_;

    VkCommandPool commandPool = VK_NULL_HANDLE;


    void create_instance();

    void create_surface();

    bool choose_one_physical_device();

    uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

    void create_device();

    void create_VMA();

    void create_swap_chain(VkSwapchainKHR old_swap_chain);

    void create_depth_resources();

    void create_swap_chain_image_view();

    void create_depth_image_view();

    void create_command_pool() {
        // Command pool
        VkCommandPoolCreateInfo commandPoolCI{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = get_queue_Family()
        };
        VK_CHECK_RESULT(vkCreateCommandPool(get_device(), &commandPoolCI, nullptr, &commandPool));
    }

    void init_Descriptor_Pool() {
        static constexpr uint32_t POOL_SIZE_DESCRIPTOR_SETS = 250;

        static constexpr uint32_t POOL_SIZE_STORAGE_BUFFER         = 1000;
        static constexpr uint32_t POOL_SIZE_STORAGE_IMAGE          = 250;
        static constexpr uint32_t POOL_SIZE_COMBINED_IMAGE_SAMPLER = 250;
        static constexpr uint32_t POOL_SIZE_UNIFORM_BUFFER         = 500;
        static constexpr uint32_t POOL_SIZE_UNIFORM_TEXEL_BUFFER   = 100;
        static constexpr uint32_t POOL_SIZE_INPUT_ATTACHMENT       = 100;

        std::vector<VkDescriptorPoolSize> pool_sizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, POOL_SIZE_STORAGE_BUFFER},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, POOL_SIZE_STORAGE_IMAGE},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, POOL_SIZE_COMBINED_IMAGE_SAMPLER},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, POOL_SIZE_UNIFORM_BUFFER},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, POOL_SIZE_UNIFORM_TEXEL_BUFFER},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, POOL_SIZE_INPUT_ATTACHMENT}
        };


        // 可以参考 blender 中是如何分配的，blender 中有具体的预分配 类型 与 数值
        VkDescriptorPoolCreateInfo descPoolCI{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets       = POOL_SIZE_DESCRIPTOR_SETS,
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes    = pool_sizes.data(),
        };
        VK_CHECK_RESULT(vkCreateDescriptorPool(get_device(), &descPoolCI, nullptr, &descriptorPool));
    }

public:
    static VK_handle &get();

    void destroy_descriptorPool() const {
        vkDestroyDescriptorPool(get_device(), descriptorPool, nullptr);
    }

    /**
     * 顺序不能更改
     */
    void init_device_handle() {
        create_instance();
        create_surface();
        choose_one_physical_device();
        create_device();
        create_VMA();
        create_swap_chain(VK_NULL_HANDLE);
        create_swap_chain_image_view();
        create_depth_image_view();
        create_command_pool();
        init_Descriptor_Pool();
    }

    void recreate_swap_chain() {
        std::cout << "recreate_swap_chain" << std::endl;
        framebufferResized = false;
        vkDeviceWaitIdle(device_);
        const auto old_swap_chain = swap_chain_;
        create_swap_chain(old_swap_chain);
        vmaDestroyImage(allocator_, depth_image_, depthImageAllocation);
        vkDestroyImageView(device_, depth_image_view_, nullptr);
        for (auto i = 0; i < swap_chain_image_views_.size(); i++) {
            vkDestroyImageView(device_, swap_chain_image_views_[i], nullptr);
        }
        vkDestroySwapchainKHR(device_, old_swap_chain, nullptr);

        create_swap_chain_image_view();
        create_depth_image_view();
    }

    void destroy();

    [[nodiscard]] VkExtent2D get_current_extent() const {
        const VkExtent2D extent = get_swap_image_rational_extent(physical_device_, surface_, window_);
        return extent;
    }

    [[nodiscard]] VkViewport get_viewport() const {
        auto temp_extent = get_current_extent();

        VkViewport viewport{
            .width    = static_cast<float>(temp_extent.width),
            .height   = static_cast<float>(temp_extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        return viewport;
    }

    [[nodiscard]] VkRect2D get_scissor() const {
        const auto temp_extent = get_current_extent();
        VkRect2D scissor{
            .extent = temp_extent,
        };
        return scissor;
    }

    [[nodiscard]] const VkFormat &get_image_format() const {
        VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(physical_device_, surface_);
        return surfaceFormat.format;
    }

    [[nodiscard]] const VkFormat &get_depth_format() const {
        return depth_format_;
    }


    VmaAllocation depthImageAllocation{}; // ????? 这是一个什么东西？

    [[nodiscard]] uint32_t get_queue_Family() const {
        return queue_family_;
    }

    [[nodiscard]] const VkInstance &get_instance() const {
        return instance_;
    }

    [[nodiscard]] const VkDevice &get_device() const {
        return device_;
    }

    [[nodiscard]] const VkQueue &get_queue() const {
        return graphics_queue_;
    }

    [[nodiscard]] const VkSurfaceKHR &get_surface() const {
        return surface_;
    }

    [[nodiscard]] const VkSwapchainKHR &get_swap_chain() const {
        return swap_chain_;
    }

    [[nodiscard]] const std::vector<VkImageView> &get_swap_image_views() const {
        return swap_chain_image_views_;
    }

    VkImage &get_depth_image() {
        return depth_image_;
    }

    [[nodiscard]] const VkImageView &get_depth_image_view() const {
        return depth_image_view_;
    }

    [[nodiscard]] const VmaAllocator &get_allocator() const {
        return allocator_;
    }

    [[nodiscard]] const GLFWwindow *get_window() const {
        return window_;
    }

    [[nodiscard]] const std::vector<VkImage> &get_swap_chain_images() const {
        return swap_chain_images_;
    }

    [[nodiscard]] VkSurfaceCapabilitiesKHR get_surface_caps() const {
        VkSurfaceCapabilitiesKHR surface_caps_{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &surface_caps_);
        return surface_caps_;
    }

private:
    VK_handle() = default;

    ~VK_handle();

public:
    VK_handle(const VK_handle &) = delete;

    VK_handle &operator=(const VK_handle &) = delete;

    VK_handle(VK_handle &&) = delete;

    VK_handle &operator=(VK_handle &&) = delete;
};

template<typename... Args>
VkDeviceAddress update_shader_data(Args... args) {
    // auto tuple             = std::make_tuple(args...);
    // constexpr size_t count = sizeof...(Args);

    uint32_t memory_size = 0;
    ([&] {
        memory_size += sizeof(args);
    }(), ...);
    // 从内存中分配
    uint64_t memory_offset = 0;

    auto start_address = reinterpret_cast<char *>(VK_handle::get().get_current_shader_data_buffer().
        get_point_mapped_address());
    ([&] {
        std::copy_n(reinterpret_cast<const char *>(&args), sizeof(args), start_address + memory_offset);
        memory_offset += sizeof(args);
    }(), ...);
    return VK_handle::get().get_current_shader_data_buffer().deviceAddress;
}

#endif //HELLO_MAC_GLFW_VULKAN_H

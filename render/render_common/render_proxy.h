//
// Created by 潘鑫 on 2026/3/3.
//

#ifndef HELLO_MAC_RENDER_PROXY_H
#define HELLO_MAC_RENDER_PROXY_H

// class VKR_object_proxy {
// public:
//     std::string pass_name;
//     std::string debug_name;
//     VkPipeline vk_pipeline;
//     VkPipelineLayout pipeline_layout;
//     std::vector<DescriptorSet_ptr> vk_descriptor_sets; // descriptor_set 的 共享指针保存点
//     VkViewport viewport;
//     VkRect2D scissor;
//     std::byte push_constants_pool[128]; // 这里有点问题，缺内容了
//     std::optional<float> line_width;
//     std::vector<VKR_Primitive> mesh;
//     // mesh 是拥有相同的模型矩阵，但是其中的 Primitive 的材质是不同的
//     // 但是还是需要共享 同一个 vk_descriptor_sets ，不同的材质部分通过索引或者其他方式的偏移来完成绘制
//     // 对不同的材质做区分，有的需要先绘制，有的之后绘制
// };

class Proxy_entity {
public:
    entt::entity entity_;
};


struct deferred_pass_tag {
};

struct volume_pass_tag {
};

class compute_group_count {
public:
    uint32_t X = 0;
    uint32_t Y = 0;
    uint32_t Z = 0;

    compute_group_count(const uint32_t x,
                        const uint32_t y,
                        const uint32_t z) {
        X = x;
        Y = y;
        Z = z;
    }
};

struct compute_pass_tag {
};

struct shadow_pass_tag {
};

struct skybox_tag {
};

struct translate_tag {
};

struct opacity_tag {
};

struct Line_tag {
};

struct gltf_tag {
};

struct skinning_tag {
};

struct bindless_tag {
};

struct Render_destroy_tag {
};


#endif //HELLO_MAC_RENDER_PROXY_H

//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_VKR_PROXY_COMPONENT_H
#define HELLO_MAC_VKR_PROXY_COMPONENT_H


#include "global_singleton.h"
#include "render_proxy.h"


bool create_VKR_object_proxy(const entt::entity entity);


using entt_proxy_update_lambda = const std::function<void(void)> &;


bool clean_VKR_object_proxy(const entt::entity entity);


void add_new_peoxy_to_render_function();

void logic_update_add_skybox_tag(const entt::entity logic_entity);


void logic_update_proxy_descriptor_sets(const entt::entity logic_entity,
                                        const std::vector<DescriptorSet_ptr> vk_descriptor_sets);

void logic_update_Mesh(const entt::entity logic_entity,
                       const std::vector<VKR_Primitive> mesh);

#endif //HELLO_MAC_VKR_PROXY_COMPONENT_H

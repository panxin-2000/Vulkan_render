//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_VKR_PROXY_COMPONENT_H
#define HELLO_MAC_VKR_PROXY_COMPONENT_H


#include "global_singleton.h"
#include "render_proxy.h"


bool create_VKR_object_proxy(const entt::entity entity);

using proxy_update_lambda = const std::function<void(std::shared_ptr<VKR_object_proxy> render_object)> &;

bool update_VKR_object_proxy(const entt::entity entity, proxy_update_lambda callback);

bool clean_VKR_object_proxy(const entt::entity entity);


#endif //HELLO_MAC_VKR_PROXY_COMPONENT_H

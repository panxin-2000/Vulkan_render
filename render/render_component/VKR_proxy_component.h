//
// Created by 潘鑫 on 2026/3/6.
//

#ifndef HELLO_MAC_VKR_PROXY_COMPONENT_H
#define HELLO_MAC_VKR_PROXY_COMPONENT_H


#include "global_singleton.h"
#include "render_proxy.h"
#include "vulkan_render_manage.h"


using entt_proxy_update_lambda = const std::function<void(void)> &;

bool clean_VKR_object_proxy(const entt::entity entity);


/**
 * 这个函数的逻辑应该是将固定区域内的内容，添加到渲染线程之中，
 */
void add_proxy_to_render_function();


inline entt::entity get_proxy_entity(const entt::entity logic_entity) {
    if (const auto render_entity = Logic_entt().try_get<Proxy_entity>(logic_entity)) {
        return render_entity->entity_;
    }
    auto value = Render_entt().create();
    Logic_entt().emplace<Proxy_entity>(logic_entity, value);
    return value;
}

template<typename T>
void logic_update_proxy(const entt::entity logic_entity, const T data) {
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity, data ]() {
            Render_entt().emplace_or_replace<T>(proxy_entity, data);
        };
        vk_render_queue::instance().render_update_entt(lambda);
    };
}

inline void logic_create_proxy(const entt::entity logic_entity) {
    Logic_entt().emplace<Proxy_entity>(logic_entity, Render_entt().create());
}

template<typename T>
void logic_update_proxy(const entt::entity logic_entity) {
    if (auto data = Logic_entt().try_get<T>(logic_entity))
        logic_update_proxy(logic_entity, *data);
}

template<typename T>
void logic_update_add_tag(const entt::entity logic_entity /*tag*/) {
    Logic_entt().emplace_or_replace<T>(logic_entity);
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity ]() {
            Render_entt().emplace_or_replace<T>(proxy_entity);
        };
        vk_render_queue::instance().render_update_entt(lambda);
    }
}

template<typename T>
void logic_update_remove_tag(const entt::entity logic_entity) {
    Logic_entt().remove<T>(logic_entity);
    if (auto proxy_entity = get_proxy_entity(logic_entity); proxy_entity != entt::null) {
        auto lambda = [ proxy_entity ]() {
            Render_entt().remove<T>(proxy_entity);
        };
        vk_render_queue::instance().render_update_entt(lambda);
    }
}

#endif //HELLO_MAC_VKR_PROXY_COMPONENT_H

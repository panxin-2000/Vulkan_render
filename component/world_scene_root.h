//
// Created by 潘鑫 on 2026/7/27.
//

#ifndef HELLO_MAC_WORLD_SCENE_ROOT_H
#define HELLO_MAC_WORLD_SCENE_ROOT_H
#include <scene_component.h>


void init_world_scene_root(entt::entity entity);

class world_scene_root {
public:
    // 获取全局唯一的注册表引用
    static entt::entity &get() {
        static entt::entity instance = Logic_entt().create();;
        static std::once_flag flag;
        std::call_once(flag, []() {
                           init_world_scene_root(instance);
                       }
                      );
        return instance;
    }

private:
    world_scene_root() = default; // 禁用构造
};


inline entt::entity &get_world_root() {
    return world_scene_root::get();
}


Ray<Point_3> &get_screen_ray(const Point_2 mouse_positon);


wmOperatorStatus model_3d_Event(const entt::entity entity, const SDL_Event &event);


uint32_t free_bindless_uniform_sampler2D(const std::string &name);



#endif //HELLO_MAC_WORLD_SCENE_ROOT_H

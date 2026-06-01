//
// Created by 潘鑫 on 2026/3/8.
//


#include "transform_component.h"
#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_render_manage.h"


inline void update_object_transform_function() { {
        const auto view = Logic_entt().view<UI_transform_dirty, Proxy_entity, Rect_2D_transform>();
        // 包围盒发生了更新
        for (const auto it: view) {
            auto pos    = view.get<Rect_2D_transform>(it);
            auto offset = pos.get_offset();
            matrix_4x4 view;
            UI_matrix_4x4(&view, {1, 1}, pos.get_offset());
            set_render_parameter(it, "model_4x4", view); // 这里直接设置有问题，到渲染线程之后再设置
            Logic_entt().remove<UI_transform_dirty>(it);
        }
    } {
        const auto view = Logic_entt().view<UI_transform_dirty, Proxy_entity, Transform>();
        for (const auto it: view) {
            auto &transform  = view.get<Transform>(it);
            auto modelMatrix = get_model_matrix(transform);
            set_render_parameter(it, "model_4x4", modelMatrix);
            Logic_entt().remove<UI_transform_dirty>(it);
        }
    }
}



void sync_render_data_to_render_thread() {
    // 应该不止更新 position，还有很多的都需要更新
    // 其实下面的两个也不应该这样写
    update_camera_transform();
    update_object_transform_function();

    // 中间这部分需要移动



    add_new_proxy_to_render();
}


bool clean_VKR_object_proxy(const entt::entity entity) {
    if (auto render = Logic_entt().try_get<Proxy_entity>(entity)) {
        const auto entity_temp = render->entity_;
        auto lambda            = [entity_temp]() {
            Render_entt().emplace_or_replace<Render_destroy_tag>(entity_temp);
        };
        vk_render_queue::instance().render_update_entt(lambda);
        return true;
    }
    return false;
}

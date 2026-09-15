//
// Created by 潘鑫 on 2026/3/8.
//


#include "load_gltf_model.h"
#include "model_matrix.h"
#include "transform_component.h"
#include "Rect_2D_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"
#include "vulkan_render_manage.h"
#include "world_scene_root.h"


void sync_render_data_to_render_thread(float time_milliseconds) {
    // 应该不止更新 position，还有很多的都需要更新
    // 其实下面的两个也不应该这样写
    update_camera_transform();

    using namespace oneapi::tbb::flow;

    graph g;

    broadcast_node<continue_msg> start(g);
    continue_node<continue_msg> transform_matrix(g,
                                                 unlimited,
                                                 [](continue_msg) -> int {
                                                     const auto root = get_world_root();
                                                     add_recursion_function_to_children(root,
                                                              update_transform_matrix);
                                                     return 0;
                                                 });
    continue_node<continue_msg> transform_world_AABB(g,
                                                     unlimited,
                                                     [](continue_msg) -> int {
                                                         const auto root = get_world_root();
                                                         add_recursion_function_to_children(root,
                                                                  update_world_AABB);
                                                         return 0;
                                                     });
    continue_node<continue_msg> add_new_model(g,
                                              unlimited,
                                              [](continue_msg) -> int {
                                                  const auto view = Logic_entt().view<Add_new_model>();
                                                  for (const auto it: view) {
                                                      deal_new_add_model(it);
                                                      Logic_entt().remove<Add_new_model>(it);
                                                  }
                                                  return 0;
                                              });
    continue_node<continue_msg> update_animals(g,
                                               unlimited,
                                               [](continue_msg) -> int {
                                                   const auto view = Logic_entt().view<JointMatrixDirty>();
                                                   for (const auto it: view) {
                                                       gltf_update_joint_matrix(it);
                                                       Logic_entt().remove<JointMatrixDirty>(it);
                                                   }
                                               });
    continue_node<continue_msg> update_JointMatrix(g,
                                                   unlimited,
                                                   [time_milliseconds](continue_msg) -> int {
                                                       const auto view = Logic_entt().view<std::vector<
                                                           RuntimeAnimation> >();
                                                       for (const auto entity: view) {
                                                           if (auto animation = Logic_entt().try_get<std::vector<
                                                               RuntimeAnimation> >(entity)) {
                                                               // 怎么把下面这个 给到一个 时间线呢?
                                                               animation->at(0).
                                                                       apply_animation(time_milliseconds, true);
                                                               Logic_entt().emplace_or_replace<
                                                                   JointMatrixDirty>(entity);
                                                           }
                                                       }
                                                   });

    make_edge(start, transform_matrix);
    make_edge(transform_matrix, add_new_model);
    make_edge(transform_matrix, transform_world_AABB);
    make_edge(add_new_model, update_JointMatrix);
    make_edge(update_JointMatrix, update_animals);

    start.try_put(continue_msg());
    g.wait_for_all();

    // 中间这部分需要移动
    Command_submit_manager::set_sync();
    vk_render_queue::instance().logic_add_finished();
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

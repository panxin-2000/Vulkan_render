//
// Created by 潘鑫 on 2026/8/3.
//

#include "simple_computer_buffer.h"
#include "global_singleton.h"
#include "name_component.h"
#include "shader_component.h"
#include "VKR_proxy_component.h"


void add_simple_computer_buffer_write() {
    const entt::entity entity = Logic_entt().create();
    logic_create_proxy(entity);
    logic_update_proxy<Name_component>(entity);
    Logic_entt().emplace<Name_component>(entity, "computer_buffer_write");


    add_shader(entity,
               "",
               "",
               "",
               "/Users/panxin/CLionProjects/hello_mac/render/shader/simple_write_buffer.comp.spv"
              );


    Logic_entt().emplace<compute_group_count>(entity, 10, 10, 10);
    const auto group_count = Logic_entt().get<compute_group_count>(entity);
    auto temp_ptr          = create_SSBO_buffer(ALIGN_1024(sizeof(VkDrawIndexedIndirectCommand) *
                                                  group_count.X *
                                                  group_count.Y *
                                                  group_count.Z *
                                                  8 * 8 * 1));

    // 下面是设置一个参数
    set_render_parameter(entity, "IndirectDraws", temp_ptr);


    logic_update_add_tag<compute_pass_tag>(entity);
    logic_update_proxy<Name_component>(entity);
    logic_update_proxy(entity, get_VKR_mesh(entity));
    logic_update_proxy(entity, create_primitives(entity));
    logic_update_proxy<compute_group_count>(entity);
}

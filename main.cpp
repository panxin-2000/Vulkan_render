#include <iostream>

#include "windows.h"

#include "event/base_event.h"
#include "labyrinth.h"
#include "UI_block.h"
#include "include/UI_button.h"


entt::entity create_button(const std::string &name,
                           int min_x,
                           int min_y,
                           int max_x,
                           int max_y) {
    auto entity = get_entt_instance().create();
    get_entt_instance().emplace<UI_button>(entity, name, entity, min_x, min_y, max_x, max_y);
    return entity;
}


entt::entity block_add_button(entt::entity block_entity,
                              const std::string &name,
                              int min_x,
                              int min_y,
                              int max_x,
                              int max_y) {
    // auto &render = get_entt_instance().get<logic_render_data>(block_entity);
    // auto vertices = render.get_vertices();
    // auto indices = render.get_indices();
    //
    // indices->push_back(vertices->size() + 0);
    // indices->push_back(vertices->size() + 1);
    // indices->push_back(vertices->size() + 2);
    // indices->push_back(vertices->size() + 2);
    // indices->push_back(vertices->size() + 3);
    // indices->push_back(vertices->size() + 0);
    // vertices->emplace_back(min_x, min_y, 0); //0 1 2
    // vertices->emplace_back(max_x, min_y, 0);
    // vertices->emplace_back(max_x, max_y, 0); // 2 3 0
    // vertices->emplace_back(min_x, max_y, 0);
    auto entity = get_entt_instance().create();
    get_entt_instance().emplace<UI_button>(entity, name, entity, min_x, min_y, max_x, max_y);
    return block_entity;
}

int main() {
    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);


    auto block_entity = new UI_block("功能", 10, 10, 220, 220);
    block_entity->add_button("按钮1", 420, 420, 480, 480);
    block_entity->add_button("按钮2", 35, 20, 145, 130);

    // create_button("按钮2", 35, 20, 45, 30);

    add_render_windows();
}

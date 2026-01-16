#include <iostream>

#include "windows.h"

#include "event/base_event.h"
#include "labyrinth.h"
#include "include/UI_button.h"


int main() {
    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);


    auto entity_2 = get_entt_instance().create();
    get_entt_instance().emplace<UI_button>(entity_2, "按钮1", entity_2);

    add_render_windows();
    // if (get_entt_instance().valid(entity))
    // get_entt_instance().destroy(entity);
}

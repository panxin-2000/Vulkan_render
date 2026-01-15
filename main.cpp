#include <iostream>

#include "windows.h"

#include "event/base_event.h"
#include "labyrinth.h"


int main() {
    auto entity = get_entt_instance().create();
    get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);


    add_render_windows();
    if (get_entt_instance().valid(entity))
        get_entt_instance().destroy(entity);


}

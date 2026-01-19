#include <iostream>

#include "windows.h"

#include "event/base_event.h"
#include "labyrinth.h"
#include "UI/UI_block.h"
#include "UI/UI_button.h"


int main() {
    // auto entity = get_entt_instance().create();
    // get_entt_instance().emplace<Labyrinth>(entity, "迷宫", entity);


    auto block_entity = UI_block("功能块", 10, 10, 220, 220);
    add_button(block_entity, "按钮1", 420, 420, 480, 480);
    add_button(block_entity, "按钮2", 35, 20, 145, 130);

    // create_button("按钮2", 35, 20, 45, 30);

    add_render_windows();
}

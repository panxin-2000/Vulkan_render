#include <iostream>

#include "windows.h"

#include "event/base_event.h"
#include "labyrinth.h"


int main() {

    auto labyrinth = new Labyrinth("迷宫");
    auto view = get_entt_instance().view<Position_component>();
    std::cout << "View size 2 : " << view.size() << std::endl;

    add_render_windows();

    delete labyrinth;
}

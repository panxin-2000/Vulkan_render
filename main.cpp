#include <iostream>

#include "windows.h"

#include "event/base_event.h"
#include "labyrinth.h"


int main() {
    auto labyrinth = new Labyrinth("迷宫");

    add_render_windows();

    delete labyrinth;
}

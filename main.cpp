#include <iostream>

#include "labyrinth.h"
#include "windows.h"
#include "component/component.h"
#include "component/music_component.h"
#include "component/Position_component.h"

#include "event/base_event.h"
#include "event/base_observer.h"
#include "event/observer_manage.h"
#include "interactable_object/actor.h"


int main() {
    auto labyrinth = new Labyrinth("迷宫");
    add_render_windows();
    delete labyrinth;
}

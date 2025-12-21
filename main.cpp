#include <iostream>

#include "labyrinth.h"
#include "windows.h"

#include "event/base_event.h"
#include "event/base_observer.h"
#include "event/observer_manage.h"



int main() {
    auto labyrinth = new Labyrinth(); {
        base_observer<base_event> observer{EventType::key_combination, "'a'"};
        observer.set_deal_function(std::bind(&Labyrinth::run_step, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    } {
        base_observer<base_event> observer{EventType::key_combination, "'q'"};
        observer.set_deal_function(std::bind(&Labyrinth::run_init, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    } {
        base_observer<base_event> observer{EventType::MouseClick, "mouse_button_left_click"};
        observer.set_deal_function(std::bind(&Labyrinth::deal_event, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    } {
        base_observer<base_event> observer{EventType::scroll, "mouse_scroll_zoom"};
        observer.set_deal_function(std::bind(&Labyrinth::set_zoom, labyrinth, std::placeholders::_1));
        observe_manage_instance::instance().addObserver(observer);
    }
    add_render_windows();
    delete labyrinth;
}

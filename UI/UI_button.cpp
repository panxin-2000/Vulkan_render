//
// Created by 潘鑫 on 2026/3/18.
//
#include "UI/UI_button.h"

#include "input_component.h"
#include "base_event.h"
#include "render_component/Geometry_component.h"
#include "model_matrix.h"
#include "../include/name_component.h"
#include "Rect_2D_component.h"
#include "render_state.h"
#include "UI/UI_text.h"


#include  "bounding_box_2d.h"


UI_Button &UI_Button::set_Round_Rect(Eigen::Vector4f box,
                                     float right_down,
                                     float right_up,
                                     float left_down,
                                     float left_up) {
    Logic_entt().emplace_or_replace<UI_Button_data>(entity,
                                                    UI_Button_data{
                                                        box, right_down, right_up, left_down, left_up
                                                    });

    Logic_entt().emplace_or_replace<Bounding_Box_2D>(entity, box);
    return *this;
}

UI_Button &UI_Button::set_Round_Rect(float min_x,
                                     float min_y,
                                     float max_x,
                                     float max_y,
                                     float right_down,
                                     float right_up,
                                     float left_down,
                                     float left_up) {
    Logic_entt().emplace_or_replace<UI_Button_data>(entity, UI_Button_data{
                                                        {min_x, min_y, max_x, max_y},
                                                        right_down, right_up, left_down, left_up
                                                    });
    Logic_entt().emplace_or_replace<Bounding_Box_2D>(entity, Eigen::Vector4f{min_x, min_y, max_x, max_y});
    return *this;
}


UI_Button::UI_Button(const std::string &debug_name)
    : object_2d(debug_name) {
    Logic_entt().emplace_or_replace<Tag_UI_Button>(entity);
}


UI_Button &UI_Button::update() {
    Logic_entt().emplace_or_replace<Tag_UI_Button_data_dirty>(entity);
    return *this;
}

UI_Button &UI_Button::set_renderable(const bool renderable) {
    if (renderable == true) {
        Logic_entt().emplace_or_replace<Tag_UI_Button_renderable>(entity);
    } else {
        Logic_entt().remove<Tag_UI_Button_renderable>(entity);
    }
    return *this;
}


UI_Text UI_Button::add_text(const std::string &debug_name) const {
    const UI_Text text(debug_name);
    add_relation(entity, text.get_entity());
    return text;
}

UI_Button UI_Button::add_button(const std::string &debug_name) const {
    const UI_Button button("debug_name");
    add_relation(entity, button.get_entity());
    return button;
}

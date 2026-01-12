//
// Created by 潘鑫 on 2026/1/13.
//
// #include "actor.h"
// #include "component.h"
// #include "component.h"
// #include "Position_component.h"

// bool Position_component::update_position() {
    // if (const auto owner_ = get_owner(); owner_ != nullptr
    // ) {
        // auto render = owner_->get_render_component();
        // if (render != nullptr) {
            // Shader_object::data_value_or_ptr data{};
            // Shader_object::set_model_transform_zoom_rotate(data.vec_4,
                                                           // {zoom.x, zoom.y, 1.0},
                                                           // {0.0f, 0.0f, 0.0f}, {offset});
            // render->add_uniform("model_transform", Shader_object::gl_mat4, data);
        // }
    // }
// }

//
//
// Position_component *Actor::add_position_component() {
//     position = new
//     Position_component(this, "render");
//     position->initialize();
//     return position;
// }

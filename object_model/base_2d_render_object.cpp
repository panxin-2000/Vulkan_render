//
// Created by 潘鑫 on 2026/8/22.
//

#include "base_2d_render_object.h"

#include "name_component.h"
#include "scene_component.h"

object_2d::object_2d(const std::string &name) : logic_render_object(name) {
    logic_create_proxy(entity);
    // add_model_3d_Event(entity); // 这里不对,需要改为2d的
    logic_update_proxy<Name_component>(entity);
    UI_root_add_child(entity);
}

//
// Created by 潘鑫 on 2026/5/28.
//

#ifndef HELLO_MAC_UI_IMGUI_H
#define HELLO_MAC_UI_IMGUI_H
#include "imgui.h"
#include "global_singleton.h"


entt::entity create_imgui_entity(const std::string &name, ImDrawData *draw_data);

entt::entity imgui_draw_new_frame(const entt::entity entity,
                                  bool &show_demo_window,
                                  bool &show_another_window,
                                  ImVec4 &clear_color);
#endif //HELLO_MAC_UI_IMGUI_H

//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_UI_BUTTON_H
#define HELLO_MAC_UI_BUTTON_H


#include "span.hpp"
#include "global_singleton.h"

// 按键和鼠标有两种截然不同的策略，基本上，所以的鼠标的点击时并没有反应，但是呢？
// 松开时 采取执行按键设计的动作

// 键盘是另一种操作，按下时就去执行响应的动作，有时会增加弹窗来进行确认


entt::entity UI_button(const std::string &name,
                       float min_x,
                       float min_y,
                       float max_x,
                       float max_y);

#endif //HELLO_MAC_UI_BUTTON_H

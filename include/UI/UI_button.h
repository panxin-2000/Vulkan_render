//
// Created by 潘鑫 on 2026/1/16.
//

#ifndef HELLO_MAC_UI_BUTTON_H
#define HELLO_MAC_UI_BUTTON_H


#include "base_2d_render_object.h"
#include "span.hpp"
#include "global_singleton.h"
#include "UI_text.h"

// 按键和鼠标有两种截然不同的策略，基本上，所以的鼠标的点击时并没有反应，但是呢？
// 松开时 采取执行按键设计的动作

// 键盘是另一种操作，按下时就去执行响应的动作，有时会增加弹窗来进行确认


struct UI_Button_data {
    Eigen::Vector4f box;
    float right_down, right_up, left_down, left_up;

    Eigen::Vector4f get_render_bounding_box() const {
        return Eigen::Vector4f(box[0] + std::max(left_down, left_up),
                               box[1] + +std::max(right_up, left_up),
                               box[2] - std::max(left_down, left_up),
                               box[3] - std::max(right_up, left_up));
    }

    Eigen::Vector4f get_interactive_bounding_box() const {
        return Eigen::Vector4f(box[0] - std::max(left_down, left_up),
                               box[1] - +std::max(right_up, left_up),
                               box[2] + std::max(left_down, left_up),
                               box[3] + std::max(right_up, left_up));
    }
};


struct Tag_UI_Button {
};

struct Tag_UI_Button_data_dirty {
};

struct Tag_UI_Button_renderable {
};


class UI_Button : public object_2d {
public:
    UI_Button &set_Round_Rect(float min_x, float min_y, float max_x, float max_y,
                              float right_down = 10, float right_up = 10, float left_down = 10, float left_up = 10);


    UI_Button &set_Round_Rect(Eigen::Vector4f box,
                              float right_down,
                              float right_up,
                              float left_down,
                              float left_up);

    UI_Button(const std::string &debug_name);

    void draw() const;

    void update() const;

    /**
     * 其全部子节点都会不再绘制
     * @param renderable true or false
     */
    void set_renderable(bool renderable) const;

    UI_Text add_text(const std::string &debug_name) const;

    UI_Button add_button(const std::string &debug_name) const;
};

void set_round_box_impl(entt::entity entity,
                        float min_x,
                        float min_y,
                        float max_x,
                        float max_y);

void set_round_box_impl(entt::entity entity,
                        const Eigen::Vector4f &box);

// entt::entity UI_button(const std::string &name,
// float min_x,
// float min_y,
// float max_x,
// float max_y);

#endif //HELLO_MAC_UI_BUTTON_H

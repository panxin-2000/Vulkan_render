//
// Created by 潘鑫 on 2025/12/23.
//

#ifndef HELLO_MAC_RENDER_COMPONENT_H
#define HELLO_MAC_RENDER_COMPONENT_H
#include "base_event.h"
#include "base_element/point_3.h"
#include "component.h"
#include "shader.h"


class Position_component : public Actor_component {
public:
    Position_component(Actor *owner, const std::string &compName)
        : Actor_component(owner, compName) {
    }

    // 重写初始化：模拟加载模型
    void initialize() override {
        if (bIsActive) {
            std::cout << "位置组件 [" << component_name << "]" << std::endl;
        }
    }

    // 自定义功能：设置模型可见性
    void set_visibility(bool bVisible) {
        std::cout << "位置组件 [" << component_name << "] " << (bVisible ? "显示" : "隐藏") << "模型" << std::endl;
    }

    bool set_zoom(const base_event_with_stamp &base_event) {
        zoom.x = zoom.x * std::powf(1.5, base_event.data.scroll.x * 0.01);
        zoom.y = zoom.y * std::powf(1.5, base_event.data.scroll.y * 0.01);
    }

    bool set_position_offset(const base_event_with_stamp &base_event) {
        const base_event_with_stamp::Drag &drag = base_event.data.drag;
        offset = offset + drag.skew;
        return true;
    }

    bool update_position();

private
:
    Point_2 zoom = {1, 1};
    Point_2 offset = {0, 0};
};

#endif //HELLO_MAC_RENDER_COMPONENT_H

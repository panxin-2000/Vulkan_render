//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_EVENT_BASE_H
#define LEARN_OPENGL_EVENT_BASE_H
#include <string>

#include "base_element/point_2.h"
#include "base_element/geometry/AABB_bounding_box.h"

// 事件类型枚举（扩展时新增枚举值即可）
enum class EventType : uint32_t {
    mouse_click_left,    // 鼠标点击事件
    mouse_release_left,  // 鼠标点击事件
    mouse_click_right,   // 鼠标点击事件
    mouse_release_right, // 鼠标点击事件
    scroll,              // 鼠标滚动缩放事件
    drag,                // 鼠标拖动事件
    area_select,         // 鼠标点击事件
    // 上面的都是需要AABB包围盒的，问题是我要AABB包围盒放在哪里？
    // 应该放置在
    key_combination, // 组合键事件
    WindowResize,    // 窗口大小变化事件
    EventType_max
};


struct Drag_event {
};


// 鼠标拖动事件
// 这个事件其实是稍微有点难评的
// 需要传入首次点击的位置
// 然后还需要添加偏移的位置
// 拖动是一个连续的事件，需要再次判断点击的位置，然后再次更新偏移的位置，
// 一直循环，知道松开


// 通用事件基类
struct base_event {
    EventType type = EventType::EventType_max; // 事件类型
    std::string event_name;                    // 事件名称

    base_event() = default;

    base_event(EventType t, const std::string &event_name)
        : type(t), event_name(event_name) {
    }

    bool operator==(const base_event &right) const {
        return type == right.type && event_name == right.event_name;
    }

    virtual ~base_event() = default; // 虚析构保证派生类析构
};

using mouse_position = Point_2;


// 通用事件基类
struct base_event_with_stamp : public base_event {
    std::chrono::milliseconds timestamp{0}; // 事件时间戳（高精度）

    struct Drag {
        Point_2 start_pos;
        Point_2 skew;
    };

    struct click_and_release_pos {
        Point_2 click_pos;
        Point_2 release_pos;
    };

    union message_data {
        mouse_position pos;
        click_and_release_pos select_box;
        mouse_position scroll;
        Drag drag;
    };

    message_data data;

    base_event_with_stamp() = default;

    base_event_with_stamp(const EventType t, const std::string &event_name)
        : base_event(t, event_name), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
    }

    base_event_with_stamp(const EventType t, const std::string &event_name, mouse_position pos)
        : base_event(t, event_name), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
        data.pos = pos;
    }

    base_event_with_stamp(const EventType t, const std::string &event_name, Drag drag)
        : base_event(t, event_name), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
        data.drag = drag;
    }

    virtual ~base_event_with_stamp() = default; // 虚析构保证派生类析构
};


#endif //LEARN_OPENGL_EVENT_BASE_H

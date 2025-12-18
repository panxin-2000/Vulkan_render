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
    key_combination, // 组合键事件
    MouseClick, // 鼠标点击事件
    area_select, // 鼠标点击事件
    WindowResize, // 窗口大小变化事件
    EventType_max
};

// 通用事件基类
struct base_event {
    EventType type = EventType::EventType_max; // 事件类型
    std::string event_name; // 事件名称

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


    union message_data {
        mouse_position pos;
        AABB_centroid<Point_2> select_box;
    };

    message_data data;

    base_event_with_stamp() = default;

    base_event_with_stamp(EventType t, const std::string &event_name)
        : base_event(t, event_name), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
    }

    base_event_with_stamp(EventType t, const std::string &event_name, mouse_position pos)
        : base_event(t, event_name), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
        data.pos = pos;
    }

    base_event_with_stamp(EventType t, const std::string &event_name, AABB_centroid<Point_2> select_box)
        : base_event(t, event_name), timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())) {
        data.select_box = select_box;
    }

    virtual ~base_event_with_stamp() = default; // 虚析构保证派生类析构
};


#endif //LEARN_OPENGL_EVENT_BASE_H

//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_EVENT_BASE_H
#define LEARN_OPENGL_EVENT_BASE_H
#include <string>


#include "utility.h"

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

enum wmEventType : int16_t {
    /* Non-event, for example disabled timer. */
    EVENT_NONE = 0x0000,
    EVENT_FIRST_LEFT,
    EVENT_PRESS_DOWN_LEFT,
    EVENT_RELEASE_LEFT,
    EVENT_FIRST_RIGHT,
    EVENT_PRESS_DOWN_RIGHT,
    EVENT_RELEASE_RIGHT,
    EVENT_SCROLL,
    EVENT_MOVE,
};


enum wmEventModifierFlag : uint8_t {
    KM_NULL  = 0,
    KM_SHIFT = (1 << 0),
    KM_CTRL  = (1 << 1),
    KM_ALT   = (1 << 2),
    /** Use for Windows-Key on MS-Windows, Command-key on macOS and Super on Linux. */
    KM_OS_KEY = (1 << 3),
    KM_HYPER  = (1 << 4),
};

ENABLE_BITWISE_OPERATORS(wmEventModifierFlag)


struct Drag_event {
};


/**
 * Operator type return flags: exec(), invoke() modal(), return values.
 */
enum wmOperatorStatus {
    OPERATOR_ZERO          = 0,
    OPERATOR_RUNNING_MODAL = (1 << 0),
    OPERATOR_CANCELLED     = (1 << 1),
    OPERATOR_FINISHED      = (1 << 2),
    /** Add this flag if the event should pass through. */
    OPERATOR_PASS_THROUGH = (1 << 3),
    /** In case operator got executed outside WM code (like via file-select). */
    OPERATOR_HANDLED = (1 << 4),
    /**
     * Used for operators that act indirectly (eg. popup menu).
     * \note this isn't great design (using operators to trigger UI) avoid where possible.
     */
    OPERATOR_INTERFACE = (1 << 5),
};

ENABLE_BITWISE_OPERATORS(wmOperatorStatus)


/**
 * 通用事件基类
 * mouse_position 返回的否是 0~width 和 0~height
 */
class base_event_with_stamp {
public:
    std::chrono::milliseconds timestamp{0}; // 事件时间戳（高精度）
    wmEventType event_type;
    std::array<float, 2> current_position; // 当前的鼠标位置
    std::array<float, 2> last_position;
    std::array<float, 2> click_position;
    std::array<float, 2> scroll;
    wmEventModifierFlag modifier_flag;

    base_event_with_stamp() = default;

    base_event_with_stamp(const wmEventType event_type,
                          std::array<float, 2> current_position,
                          std::array<float, 2> move_position,
                          std::array<float, 2> click_position,
                          std::array<float, 2> scroll,
                          wmEventModifierFlag modifier_flag)
        : timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                          std::chrono::system_clock::now().
                                                                          time_since_epoch())),
          event_type(event_type),
          current_position(current_position),
          last_position(move_position),
          click_position(click_position),
          scroll(scroll),
          modifier_flag(modifier_flag) {
    }

    ~base_event_with_stamp() = default; // 虚析构保证派生类析构
};


#endif //LEARN_OPENGL_EVENT_BASE_H

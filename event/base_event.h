//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_EVENT_BASE_H
#define LEARN_OPENGL_EVENT_BASE_H
#include <string>

#include "base_element/point_2.h"
#include "base_element/geometry/AABB_bounding_box.h"
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

    /* ********** Start of Input devices. ********** */

    /* Minimum mouse value (inclusive). */
#define _EVT_MOUSE_MIN 0x0001

    /* MOUSE: 0x000x, 0x001x. */
    MOUSE_LEFT   = 0x0001,
    MOUSE_MIDDLE = 0x0002,
    MOUSE_RIGHT  = 0x0003,
    MOUSE_MOVE   = 0x0004,
    /* Extra mouse buttons. */
    BUTTON4_MOUSE = 0x0007,
    BUTTON5_MOUSE = 0x0008,
    /* More mouse buttons - can't use 9 and 10 here (wheel). */
    BUTTON6_MOUSE = 0x0012,
    BUTTON7_MOUSE = 0x0013,
    /* Extra trackpad gestures (check #WM_EVENT_IS_CONSECUTIVE to detect motion events). */
    MOUSE_PAN       = 0x000e,
    MOUSE_ZOOM      = 0x000f,
    MOUSE_ROTATE    = 0x0010,
    MOUSE_SMARTZOOM = 0x0017,

    /* Defaults from ghost. */
    WHEEL_UP_MOUSE   = 0x000a,
    WHEEL_DOWN_MOUSE = 0x000b,
    /* Mapped based on #USER_WHEELZOOMDIR. */
    WHEEL_IN_MOUSE  = 0x000c,
    WHEEL_OUT_MOUSE = 0x000d,
    /* Successive MOUSEMOVE's are converted to this, so we can easily
     * ignore all but the most recent MOUSEMOVE (for better performance),
     * paint and drawing tools however will want to handle these. */
    INBETWEEN_MOUSEMOVE = 0x0011,


    EVT_KEY_SPACE_KEY  = 32,
    EVT_KEY_APOSTROPHE = 39,
    EVT_KEY_COMMA      = 44,
    EVT_KEY_MINUS      = 45,
    EVT_KEY_PERIOD     = 46,
    EVT_KEY_SLASH      = 47,


    EVT_KEY_0         = 48,
    EVT_KEY_1         = 49,
    EVT_KEY_2         = 50,
    EVT_KEY_3         = 51,
    EVT_KEY_4         = 52,
    EVT_KEY_5         = 53,
    EVT_KEY_6         = 54,
    EVT_KEY_7         = 55,
    EVT_KEY_8         = 56,
    EVT_KEY_9         = 57,
    EVT_KEY_SEMICOLON = 59,
    EVT_KEY_EQUAL     = 61,

    EVT_KEY_A = 65, /* 'a' (97). */
    EVT_KEY_B = 66, /* 'b' (98). */
    EVT_KEY_C = 67, /* 'c' (99). */
    EVT_KEY_D = 68, /* 'd' (100). */
    EVT_KEY_E = 69, /* 'e' (101). */
    EVT_KEY_F = 70, /* 'f' (102). */
    EVT_KEY_G = 71, /* 'g' (103). */
    EVT_KEY_H = 72, /* 'h' (104). */
    EVT_KEY_I = 73, /* 'i' (105). */
    EVT_KEY_J = 74, /* 'j' (106). */
    EVT_KEY_K = 75, /* 'k' (107). */
    EVT_KEY_L = 76, /* 'l' (108). */
    EVT_KEY_M = 77, /* 'm' (109). */
    EVT_KEY_N = 78, /* 'n' (110). */
    EVT_KEY_O = 79, /* 'o' (111). */
    EVT_KEY_P = 80, /* 'p' (112). */
    EVT_KEY_Q = 81, /* 'q' (113). */
    EVT_KEY_R = 82, /* 'r' (114). */
    EVT_KEY_S = 83, /* 's' (115). */
    EVT_KEY_T = 84, /* 't' (116). */
    EVT_KEY_U = 85, /* 'u' (117). */
    EVT_KEY_V = 86, /* 'v' (118). */
    EVT_KEY_W = 87, /* 'w' (119). */
    EVT_KEY_X = 88, /* 'x' (120). */
    EVT_KEY_Y = 89, /* 'y' (121). */
    EVT_KEY_Z = 90, /* 'z' (122). */

    EVT_KEY_LEFT_BRACKET  = 91,
    EVT_KEY_BACKSLASH     = 92,
    EVT_KEY_RIGHT_BRACKET = 93,
    EVT_KEY_GRAVE_ACCENT  = 96,
    EVT_KEY_WORLD_1       = 161,
    EVT_KEY_WORLD_2       = 162,


    EVT_KEY_ESCAPE       = 256,
    EVT_KEY_ENTER        = 257,
    EVT_KEY_TAB          = 258,
    EVT_KEY_BACKSPACE    = 259,
    EVT_KEY_INSERT       = 260,
    EVT_KEY_DELETE       = 261,
    EVT_KEY_RIGHT        = 262,
    EVT_KEY_LEFT         = 263,
    EVT_KEY_DOWN         = 264,
    EVT_KEY_UP           = 265,
    EVT_KEY_PAGE_UP      = 266,
    EVT_KEY_PAGE_DOWN    = 267,
    EVT_KEY_HOME         = 268,
    EVT_KEY_END          = 269,
    EVT_KEY_CAPS_LOCK    = 280,
    EVT_KEY_SCROLL_LOCK  = 281,
    EVT_KEY_NUM_LOCK     = 282,
    EVT_KEY_PRINT_SCREEN = 283,
    EVT_KEY_PAUSE        = 284,

    EVT_KEY_F1  = 290,
    EVT_KEY_F2  = 291,
    EVT_KEY_F3  = 292,
    EVT_KEY_F4  = 293,
    EVT_KEY_F5  = 294,
    EVT_KEY_F6  = 295,
    EVT_KEY_F7  = 296,
    EVT_KEY_F8  = 297,
    EVT_KEY_F9  = 298,
    EVT_KEY_F10 = 299,
    EVT_KEY_F11 = 300,
    EVT_KEY_F12 = 301,
    EVT_KEY_F13 = 302,
    EVT_KEY_F14 = 303,
    EVT_KEY_F15 = 304,
    EVT_KEY_F16 = 305,
    EVT_KEY_F17 = 306,
    EVT_KEY_F18 = 307,
    EVT_KEY_F19 = 308,
    EVT_KEY_F20 = 309,
    EVT_KEY_F21 = 310,
    EVT_KEY_F22 = 311,
    EVT_KEY_F23 = 312,
    EVT_KEY_F24 = 313,
    EVT_KEY_F25 = 314,


    EVT_KEY_PAD0 = 320,
    EVT_KEY_PAD1 = 321,
    EVT_KEY_PAD2 = 322,
    EVT_KEY_PAD3 = 323,
    EVT_KEY_PAD4 = 324,
    EVT_KEY_PAD5 = 325,
    EVT_KEY_PAD6 = 326,
    EVT_KEY_PAD7 = 327,
    EVT_KEY_PAD8 = 328,
    EVT_KEY_PAD9 = 329,
    /* Key-pad keys. */

    EVT_KEY_KP_DECIMAL    = 330,
    EVT_KEY_KP_DIVIDE     = 331,
    EVT_KEY_KP_MULTIPLY   = 332,
    EVT_KEY_KP_SUBTRACT   = 333,
    EVT_KEY_KP_ADD        = 334,
    EVT_KEY_KP_ENTER      = 335,
    EVT_KEY_KP_EQUAL      = 336,
    EVT_KEY_LEFT_SHIFT    = 340,
    EVT_KEY_LEFT_CONTROL  = 341,
    EVT_KEY_LEFT_ALT      = 342,
    EVT_KEY_LEFT_SUPER    = 343,
    EVT_KEY_RIGHT_SHIFT   = 344,
    EVT_KEY_RIGHT_CONTROL = 345,
    EVT_KEY_RIGHT_ALT     = 346,
    EVT_KEY_RIGHT_SUPER   = 347,
    EVT_KEY_MENU          = 348,
};


enum wmEventModifierFlag : uint8_t {
    KM_SHIFT = (1 << 0),
    KM_CTRL  = (1 << 1),
    KM_ALT   = (1 << 2),
    /** Use for Windows-Key on MS-Windows, Command-key on macOS and Super on Linux. */
    KM_OSKEY = (1 << 3),
    KM_HYPER = (1 << 4),
};

struct Drag_event {
};

using mouse_position = Point_2;

enum Event_code {
    KM_ANY       = -1,
    KM_NOTHING   = 0, // 没事
    KM_PRESS     = 1, // 刚刚按下时触发一次
    KM_RELEASE   = 2, // 释放时触发的
    KM_CLICK     = 3, // 下面这两个需要的处理稍微有点多
    KM_DBL_CLICK = 4, //
    /**
     * \note The cursor location at the point dragging starts is set to #wmEvent.prev_press_xy
     * some operators such as box selection should use this location instead of #wmEvent.xy.
     */
    KM_PRESS_DRAG = 5,
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
    Event_code event_code;           // 用于标记当前鼠标或者键盘的某个按键是什么状态
    mouse_position current_position; // 当前的鼠标位置
    mouse_position last_position;
    mouse_position click_position;
    Point_2 scroll;
    wmEventModifierFlag modifier_flag;

    base_event_with_stamp() = default;

    base_event_with_stamp(const wmEventType event_type,
                          Event_code event_code,
                          mouse_position current_position,
                          mouse_position move_position,
                          mouse_position click_position,
                          Point_2 scroll,
                          wmEventModifierFlag modifier_flag)
        : timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                          std::chrono::system_clock::now().
                                                                          time_since_epoch())),
          event_type(event_type),
          event_code(event_code),
          current_position(current_position),
          last_position(move_position),
          click_position(click_position),
          scroll(scroll),
          modifier_flag(modifier_flag) {
    }

    ~base_event_with_stamp() = default; // 虚析构保证派生类析构
};


#endif //LEARN_OPENGL_EVENT_BASE_H

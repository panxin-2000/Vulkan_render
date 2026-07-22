//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_EVENT_BASE_H
#define LEARN_OPENGL_EVENT_BASE_H
#include <string>


#include "utility.h"

// 事件类型枚举（扩展时新增枚举值即可）


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


#endif //LEARN_OPENGL_EVENT_BASE_H

//
// Created by 潘鑫 on 2025/12/15.
//

#ifndef LEARN_OPENGL_KEYCOMBOOBSERVER_H
#define LEARN_OPENGL_KEYCOMBOOBSERVER_H
#include <iostream>

// 观察者抽象基类
template<typename T_event>
class base_observer : public T_event {
public:
    base_observer() = default;

    ~base_observer() = default;

    base_observer(EventType t, const std::string &event_name)
        : T_event(t, event_name) {
    }

    using observer_event_type = T_event;


    void set_deal_function(std::function<void(base_event_with_stamp)> function) {
        on_Event = function;
    }


    // 组合键事件回调接口
    std::function<void(base_event_with_stamp)> on_Event;
};


#endif //LEARN_OPENGL_KEYCOMBOOBSERVER_H
